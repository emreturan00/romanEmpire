

//Author : Emre Turan - 20191709010

// This program simulates a manufacturing process with multiple stages. 
// The process consists of raw material handling, machining, assembly, quality control, and packaging stages. 
// The simulation includes machine failures and maintenance events. The simulation runs for a fixed time duration (MAX_TIME) 
// and schedules events based on the current time and the type of event. The simulation uses a priority queue to store events 
// and processes events in order of their scheduled time. The simulation generates random times for each stage based on normal 
// distributions with mean and standard deviation values. The simulation also schedules machine failures and maintenance events 
// based on exponential distributions. 
// The simulation outputs log messages for each event type and the time at which the event occurs.


#include <iostream>
#include <queue>
#include <random>
#include <vector>
#include <functional>
#include <map>
#include <string>


using namespace std;

const int MAX_TIME = 100;

enum EventType { ARRIVAL, END_RAW_MATERIAL, END_MACHINING, END_ASSEMBLY, END_QUALITY_CONTROL, END_PACKAGING, SETUP_CHANGE, MACHINE_FAILURE, MAINTENANCE };

struct Event {
    int time;
    EventType type;
    int productType;
};

struct CompareEvent {
    bool operator()(Event const& e1, Event const& e2) {
        return e1.time > e2.time;
    }
};

priority_queue<Event, vector<Event>, CompareEvent> eventQueue;

void scheduleEvent(int time, EventType type, int productType);
void processEvent(Event e, default_random_engine& generator, vector<normal_distribution<double>>& rawMaterialTimes, vector<normal_distribution<double>>& machiningTimes, vector<normal_distribution<double>>& assemblyTimes, vector<normal_distribution<double>>& qualityControlTimes, vector<normal_distribution<double>>& packagingTimes, vector<int>& machineCounts, map<int, int>& machineSetups, int& currentSetup, int setupTime, int shiftEndTime, bool& maintenanceScheduled);
void scheduleMachineFailure(int currentTime, default_random_engine& generator);
void scheduleMaintenance(int currentTime, int maintenanceInterval);

int main() {
    default_random_engine generator;
    generator.seed();
    exponential_distribution<double> arrivalDistribution(1.0 / 4.5);

    int rawMaterialMachines, machiningMachines, assemblyMachines, qualityControlMachines, packagingMachines, setupTime, maintenanceInterval, shiftLength;
    cout << "Enter the number of machines for raw material handling: ";
    cin >> rawMaterialMachines;
    cout << "Enter the number of machines for machining: ";
    cin >> machiningMachines;
    cout << "Enter the number of machines for assembly: ";
    cin >> assemblyMachines;
    cout << "Enter the number of machines for quality control: ";
    cin >> qualityControlMachines;
    cout << "Enter the number of machines for packaging: ";
    cin >> packagingMachines;
    cout << "Enter the setup change time: ";
    cin >> setupTime;
    cout << "Enter the maintenance interval: ";
    cin >> maintenanceInterval;
    cout << "Enter the shift length: ";
    cin >> shiftLength;

    vector<int> machineCounts = { rawMaterialMachines, machiningMachines, assemblyMachines, qualityControlMachines, packagingMachines };

    vector<normal_distribution<double>> rawMaterialTimes = { normal_distribution<double>(2.0, 0.5), normal_distribution<double>(3.0, 0.6) };
    vector<normal_distribution<double>> machiningTimes = { normal_distribution<double>(3.2, 0.6), normal_distribution<double>(4.0, 0.7) };
    vector<normal_distribution<double>> assemblyTimes = { normal_distribution<double>(2.5, 0.5), normal_distribution<double>(3.5, 0.6) };
    vector<normal_distribution<double>> qualityControlTimes = { normal_distribution<double>(1.5, 0.3), normal_distribution<double>(2.0, 0.4) };
    vector<normal_distribution<double>> packagingTimes = { normal_distribution<double>(1.0, 0.2), normal_distribution<double>(1.5, 0.3) };

    map<int, int> machineSetups = { {0, 0}, {1, 1} };
    int currentSetup = -1; // -1 indicates no current setup

    scheduleEvent(static_cast<int>(arrivalDistribution(generator)), ARRIVAL, 0);
    scheduleEvent(static_cast<int>(arrivalDistribution(generator)), ARRIVAL, 1);

    scheduleMachineFailure(0, generator);
    scheduleMaintenance(0, maintenanceInterval);

    bool maintenanceScheduled = false;
    int currentTime = 0;

    while (!eventQueue.empty() && eventQueue.top().time <= MAX_TIME) {
        Event currentEvent = eventQueue.top();
        eventQueue.pop();
        currentTime = currentEvent.time;
        processEvent(currentEvent, generator, rawMaterialTimes, machiningTimes, assemblyTimes, qualityControlTimes, packagingTimes, machineCounts, machineSetups, currentSetup, setupTime, shiftLength, maintenanceScheduled);
    }

    cout << "Simulation completed.\n";
    return 0;
}

void scheduleEvent(int time, EventType type, int productType) {
    Event newEvent;
    newEvent.time = time;
    newEvent.type = type;
    newEvent.productType = productType;
    eventQueue.push(newEvent);
}

void logEvent(const string& message) {
    cout << "----------------------------------------\n";
    cout << message << endl;
    cout << "----------------------------------------\n";
}

void processEvent(Event e, default_random_engine& generator, vector<normal_distribution<double>>& rawMaterialTimes, vector<normal_distribution<double>>& machiningTimes, vector<normal_distribution<double>>& assemblyTimes, vector<normal_distribution<double>>& qualityControlTimes, vector<normal_distribution<double>>& packagingTimes, vector<int>& machineCounts, map<int, int>& machineSetups, int& currentSetup, int setupTime, int shiftEndTime, bool& maintenanceScheduled) {
    exponential_distribution<double> arrivalDistribution(1.0 / 4.5);

    switch (e.type) {
    case ARRIVAL:
        logEvent("Event: ARRIVAL\nTime: " + to_string(e.time) + "\nProduct Type: " + to_string(e.productType));
        scheduleEvent(e.time + static_cast<int>(arrivalDistribution(generator)), ARRIVAL, e.productType);
        scheduleEvent(e.time + static_cast<int>(rawMaterialTimes[e.productType](generator)) / machineCounts[0], END_RAW_MATERIAL, e.productType);
        break;
    case END_RAW_MATERIAL:
        logEvent("Event: END_RAW_MATERIAL\nTime: " + to_string(e.time) + "\nProduct Type: " + to_string(e.productType));
        if (currentSetup != e.productType) {
            scheduleEvent(e.time + setupTime, SETUP_CHANGE, e.productType);
        }
        else {
            scheduleEvent(e.time + static_cast<int>(machiningTimes[e.productType](generator)) / machineCounts[1], END_MACHINING, e.productType);
        }
        break;
    case SETUP_CHANGE:
        logEvent("Event: SETUP_CHANGE\nTime: " + to_string(e.time) + "\nProduct Type: " + to_string(e.productType));
        currentSetup = e.productType;
        scheduleEvent(e.time + static_cast<int>(machiningTimes[e.productType](generator)) / machineCounts[1], END_MACHINING, e.productType);
        break;
    case END_MACHINING:
        logEvent("Event: END_MACHINING\nTime: " + to_string(e.time) + "\nProduct Type: " + to_string(e.productType));
        scheduleEvent(e.time + static_cast<int>(assemblyTimes[e.productType](generator)) / machineCounts[2], END_ASSEMBLY, e.productType);
        break;
    case END_ASSEMBLY:
        logEvent("Event: END_ASSEMBLY\nTime: " + to_string(e.time) + "\nProduct Type: " + to_string(e.productType));
        scheduleEvent(e.time + static_cast<int>(qualityControlTimes[e.productType](generator)) / machineCounts[3], END_QUALITY_CONTROL, e.productType);
        break;
    case END_QUALITY_CONTROL:
        logEvent("Event: END_QUALITY_CONTROL\nTime: " + to_string(e.time) + "\nProduct Type: " + to_string(e.productType));
        scheduleEvent(e.time + static_cast<int>(packagingTimes[e.productType](generator)) / machineCounts[4], END_PACKAGING, e.productType);
        break;
    case END_PACKAGING:
        logEvent("Event: END_PACKAGING\nTime: " + to_string(e.time) + "\nProduct Type: " + to_string(e.productType));
        break;
    case MACHINE_FAILURE:
        logEvent("Event: MACHINE_FAILURE\nTime: " + to_string(e.time));
        // Randomly determine the repair time for the failed machine
        {
            default_random_engine repairGenerator;
            repairGenerator.seed();
            uniform_int_distribution<int> repairTimeDistribution(30, 120); // Repair time between 30 to 120 minutes
            int repairTime = repairTimeDistribution(repairGenerator);
            int repairCompletionTime = e.time + repairTime;
            logEvent("Machine failure occurred. Machine will be repaired and back online at time: " + to_string(repairCompletionTime));
            // Schedule machine repair completion event
            scheduleEvent(repairCompletionTime, MAINTENANCE, -1);
        }
        break;
    case MAINTENANCE:
        logEvent("Event: MAINTENANCE\nTime: " + to_string(e.time));
        maintenanceScheduled = false;
        // Assume maintenance takes fixed time duration
        {
            int maintenanceDuration = 60; // Maintenance duration in minutes
            int maintenanceCompletionTime = e.time + maintenanceDuration;
            logEvent("Maintenance scheduled. Maintenance will be completed at time: " + to_string(maintenanceCompletionTime));
            scheduleEvent(maintenanceCompletionTime, MAINTENANCE, -1);
        }
        break;
    }

    if (!maintenanceScheduled && e.time >= shiftEndTime) {
        scheduleEvent(e.time + shiftEndTime, MAINTENANCE, -1);
        maintenanceScheduled = true;
    }
}

void scheduleMachineFailure(int currentTime, default_random_engine& generator) {
    exponential_distribution<double> failureDistribution(1.0 / 50); // Average time between failures
    int failureTime = currentTime + static_cast<int>(failureDistribution(generator));
    scheduleEvent(failureTime, MACHINE_FAILURE, -1);
}

void scheduleMaintenance(int currentTime, int maintenanceInterval) {
    int maintenanceTime = currentTime + maintenanceInterval;
    scheduleEvent(maintenanceTime, MAINTENANCE, -1);
}
