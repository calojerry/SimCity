#include <iostream>
#include <string>
#include <fstream>

#include "simulation.h"

using namespace std;

// main
int main() {
    cout << "\nSIM CITY SIMULATION - Team 1:\n" << endl;

    string configFilePath;
    cout << "\nEnter config file path: " << endl;
    getline(cin, configFilePath);

    try {
        // Create a Simulation object and initialize the simulation
        Simulation sim;
        sim.initializeSim(configFilePath);

        //Run the simulation
        sim.simulate();

    } catch (const runtime_error& e) {
        cerr << "Error during simulation: " << e.what() << endl; //cerr used for error
        return 1;
    }

    return 0;
}
