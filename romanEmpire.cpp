#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <numeric>
#include <climits>

using namespace std;

const int MAX_YEARS = 200;
const int NUM_QUAESTORS = 20;
const int NUM_AEDILES = 10;
const int NUM_PRAETORS = 8;
const int NUM_CONSULS = 2;
const int MIN_QUAESTOR_AGE = 30;
const int MIN_AEDILE_AGE = 36;
const int MIN_PRAETOR_AGE = 39;
const int MIN_CONSUL_AGE = 42;
const int REELECTION_INTERVAL = 10;
const int STARTING_PSI = 100;
const int UNFILLED_PENALTY = -5;
const int REELECTION_PENALTY = -10;
const int INITIAL_QUAESTOR_POOL = 15;
const int INITIAL_QUAESTOR_INFLUX_MEAN = 15;
const int INITIAL_QUAESTOR_INFLUX_STDDEV = 5;
const int LIFE_EXPECTANCY_MEAN = 55;
const int LIFE_EXPECTANCY_STDDEV = 10;
const int LIFE_EXPECTANCY_MIN = 25;
const int LIFE_EXPECTANCY_MAX = 80;

struct Politician {
    int age;
    bool quaestor;
    bool aedile;
    bool praetor;
    bool consul;
};

int truncated_normal_distribution(default_random_engine& generator, normal_distribution<double>& distribution, int min, int max) {
    int value;
    do {
        value = static_cast<int>(distribution(generator));
    } while (value < min || value > max);
    return value;
}

void simulate_year(vector<Politician>& politicians, int& psi, default_random_engine& generator, normal_distribution<double>& quaestor_influx_distribution) {
    for (auto& politician : politicians) {
        politician.age++;
        if (politician.age > LIFE_EXPECTANCY_MAX) {
            politician.quaestor = false;
            politician.aedile = false;
            politician.praetor = false;
            politician.consul = false;
        }
    }

    int unfilled_positions_penalty = (NUM_QUAESTORS - count_if(politicians.begin(), politicians.end(), [](const Politician& p) { return p.quaestor; })) * UNFILLED_PENALTY;
    int reelection_penalty = count_if(politicians.begin(), politicians.end(), [](const Politician& p) { return p.consul; }) / REELECTION_INTERVAL * REELECTION_PENALTY;
    psi += unfilled_positions_penalty + reelection_penalty;

    int num_new_candidates = truncated_normal_distribution(generator, quaestor_influx_distribution, 0, INT_MAX);
    for (int i = 0; i < num_new_candidates; ++i) {
        Politician new_candidate;
        new_candidate.age = 30;
        new_candidate.quaestor = true;
        politicians.push_back(new_candidate);
    }

    shuffle(politicians.begin(), politicians.end(), generator);
    int quaestors_needed = NUM_QUAESTORS - count_if(politicians.begin(), politicians.end(), [](const Politician& p) { return p.quaestor; });
    int aediles_needed = NUM_AEDILES - count_if(politicians.begin(), politicians.end(), [](const Politician& p) { return p.aedile; });
    int praetors_needed = NUM_PRAETORS - count_if(politicians.begin(), politicians.end(), [](const Politician& p) { return p.praetor; });
    int consul_needed = NUM_CONSULS - count_if(politicians.begin(), politicians.end(), [](const Politician& p) { return p.consul; });

    for (auto& politician : politicians) {
        if (quaestors_needed > 0 && politician.age >= MIN_QUAESTOR_AGE && !politician.quaestor) {
            politician.quaestor = true;
            quaestors_needed--;
        }
        else if (aediles_needed > 0 && politician.age >= MIN_AEDILE_AGE && politician.quaestor && !politician.aedile) {
            politician.aedile = true;
            aediles_needed--;
        }
        else if (praetors_needed > 0 && politician.age >= MIN_PRAETOR_AGE && politician.aedile && !politician.praetor) {
            politician.praetor = true;
            praetors_needed--;
        }
        else if (consul_needed > 0 && politician.age >= MIN_CONSUL_AGE && politician.praetor && !politician.consul) {
            politician.consul = true;
            consul_needed--;
        }
    }
}

void run_simulation() {
    random_device rd;
    default_random_engine generator(rd());
    normal_distribution<double> life_expectancy_distribution(LIFE_EXPECTANCY_MEAN, LIFE_EXPECTANCY_STDDEV);
    normal_distribution<double> quaestor_influx_distribution(INITIAL_QUAESTOR_INFLUX_MEAN, INITIAL_QUAESTOR_INFLUX_STDDEV);
    vector<Politician> politicians;
    for (int i = 0; i < NUM_QUAESTORS; ++i) {
        Politician politician;
        politician.age = MIN_QUAESTOR_AGE;
        politician.quaestor = true;
        politicians.push_back(politician);
    }
    int psi = STARTING_PSI;
    for (int year = 1; year <= MAX_YEARS; ++year) {
        simulate_year(politicians, psi, generator, quaestor_influx_distribution);
    }
    cout << "End-of-Simulation PSI: " << psi << endl;
    double quaestor_fill_rate = 0.0;
    double aedile_fill_rate = 0.0;
    double praetor_fill_rate = 0.0;
    double consul_fill_rate = 0.0;
    int total_quaestors = 0;
    int total_aediles = 0;
    int total_praetors = 0;
    int total_consuls = 0;
    for (const auto& politician : politicians) {
        if (politician.age <= LIFE_EXPECTANCY_MAX) {
            if (politician.quaestor) total_quaestors++;
            if (politician.aedile) total_aediles++;
            if (politician.praetor) total_praetors++;
            if (politician.consul) total_consuls++;
        }
    }
    quaestor_fill_rate = (NUM_QUAESTORS * MAX_YEARS - total_quaestors) / (double)(NUM_QUAESTORS * MAX_YEARS) * 100;
    aedile_fill_rate = (NUM_AEDILES * MAX_YEARS - total_aediles) / (double)(NUM_AEDILES * MAX_YEARS) * 100;
    praetor_fill_rate = (NUM_PRAETORS * MAX_YEARS - total_praetors)

        / (double)(NUM_PRAETORS * MAX_YEARS) * 100;
    consul_fill_rate = (NUM_CONSULS * MAX_YEARS - total_consuls) / (double)(NUM_CONSULS * MAX_YEARS) * 100;
    cout << "Annual Fill Rate:\n";
    cout << "Quaestor: " << quaestor_fill_rate << "%" << endl;
    cout << "Aedile: " << aedile_fill_rate << "%" << endl;
    cout << "Praetor: " << praetor_fill_rate << "%" << endl;
    cout << "Consul: " << consul_fill_rate << "%" << endl;
    vector<int> quaestor_ages;
    vector<int> aedile_ages;
    vector<int> praetor_ages;
    vector<int> consul_ages;
    for (const auto& politician : politicians) {
        if (politician.age <= LIFE_EXPECTANCY_MAX) {
            if (politician.quaestor) quaestor_ages.push_back(politician.age);
            if (politician.aedile) aedile_ages.push_back(politician.age);
            if (politician.praetor) praetor_ages.push_back(politician.age);
            if (politician.consul) consul_ages.push_back(politician.age);
        }
    }
    cout << "Age Distribution:\n";
    cout << "Quaestor: ";
    if (!quaestor_ages.empty()) {
        int min_age = *min_element(quaestor_ages.begin(), quaestor_ages.end());
        int max_age = *max_element(quaestor_ages.begin(), quaestor_ages.end());
        cout << "Min: " << min_age << ", Max: " << max_age << ", Mean: " << accumulate(quaestor_ages.begin(), quaestor_ages.end(), 0) / (double)quaestor_ages.size() << endl;
    }
    else {
        cout << "No politicians" << endl;
    }
    cout << "Aedile: ";
    if (!aedile_ages.empty()) {
        int min_age = *min_element(aedile_ages.begin(), aedile_ages.end());
        int max_age = *max_element(aedile_ages.begin(), aedile_ages.end());
        cout << "Min: " << min_age << ", Max: " << max_age << ", Mean: " << accumulate(aedile_ages.begin(), aedile_ages.end(), 0) / (double)aedile_ages.size() << endl;
    }
    else {
        cout << "No politicians" << endl;
    }
    cout << "Praetor: ";
    if (!praetor_ages.empty()) {
        int min_age = *min_element(praetor_ages.begin(), praetor_ages.end());
        int max_age = *max_element(praetor_ages.begin(), praetor_ages.end());
        cout << "Min: " << min_age << ", Max: " << max_age << ", Mean: " << accumulate(praetor_ages.begin(), praetor_ages.end(), 0) / (double)praetor_ages.size() << endl;
    }
    else {
        cout << "No politicians" << endl;
    }
    cout << "Consul: ";
    if (!consul_ages.empty()) {
        int min_age = *min_element(consul_ages.begin(), consul_ages.end());
        int max_age = *max_element(consul_ages.begin(), consul_ages.end());
        cout << "Min: " << min_age << ", Max: " << max_age << ", Mean: " << accumulate(consul_ages.begin(), consul_ages.end(), 0) / (double)consul_ages.size() << endl;
    }
    else {
        cout << "No politicians" << endl;
    }
}

int main() {
    run_simulation();

        if (__cplusplus == 202101L) std::cout << "C++23";
        else if (__cplusplus == 202002L) std::cout << "C++20";
        else if (__cplusplus == 201703L) std::cout << "C++17";
        else if (__cplusplus == 201402L) std::cout << "C++14";
        else if (__cplusplus == 201103L) std::cout << "C++11";
        else if (__cplusplus == 199711L) std::cout << "C++98";
        else std::cout << "__cplusplus: " << __cplusplus;
        std::cout << std::endl;
    
    return 0;
}
