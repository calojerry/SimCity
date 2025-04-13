#ifndef SIMULATION_H
#define SIMULATION_H

#include<string>
#include<vector>

enum ZoneType {
    RESIDENTIAL = 'R',
    COMMERCIAL = 'C',
    INDUSTRIAL = 'I',
    ROAD = '-',
    POWERLINE = 'T',
    POWERLINE_OVER_ROAD = '#',
    POWERPLANT = 'P',
    EMPTY = ' '
};

struct Config{
	std::string RegionLayout;
	int timeLimit, refreshRate;
};

struct Cell {
    ZoneType zone;
    int population = 0;
    int pollution = 0; 
    int availableWorkers = 0;
    int availableGoods = 0;
    bool isAdjacentToPowerLine = false;
    bool isPowered = false;
};

struct Stats {
    bool powerOn = false;
    int totalPopulation = 0;
    int totalGoods = 0;
    int totalWorkers = 0;
    int totalPollution = 0;
};

class Simulation{
protected:
    Config config;
    std::vector<std::vector<Cell>> regionMap;

    // functions to manip private members
    bool readConfig(const std::string& path);
    bool readRegion(const std::string& path);

	// functions to be used by child classes
    int countAdjPop(int x, int y, int minPopulation) const;

    // functions for power
    void findPowerPlant(std::vector<std::pair<int, int>>& powerPlants);
    void bfsPowerPropagation(int startX, int startY);
    void markPowered();
    bool hasAdjPower(int x, int y) const;

    // Growth rules
    void residentialGrowth();
    void commercialGrowth();
    void industrialGrowth();
    
    // Pollution management
    void spreadPollution();

    // Resource production
    void produceGoods();

    // Resource management
    int countAvailableWorkers() const;
    void assignWorkerToJob();
    int countAvailableGoods() const;         // New method to count available goods
    void assignGoodToCell();                 // New method to deduct goods
    
    // Change detection
    bool detectChanges();

public:
    Simulation();
    void initializeSim(const std::string& configFilePath);
    void updatePower();

    // printing functions
    void printConfig() const;
    void printMap() const;
    void printResults() const;

    // function that actually runs simulation
    void simulate();
};

#endif
