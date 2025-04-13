#include "simulation.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <locale>
#include <queue>

//constructor
Simulation::Simulation() {
    //default config values
    config.RegionLayout = "";
    config.timeLimit = 0;
    config.refreshRate = 0;

}

/*********************************
PROTECTED MEMBERS
*********************************/

void Simulation::initializeSim(const std::string& configFilePath) {
    // read the config file
    if (!readConfig(configFilePath)) {
        throw std::runtime_error("Failed to read the configuration file.");
    }

    // construct path to the region layout
    std::filesystem::path configPath(configFilePath);
    std::filesystem::path regionFilePath = configPath.parent_path() / config.RegionLayout;

    // use path to read region layout
    if (!readRegion(regionFilePath.string())) {
        throw std::runtime_error("Failed to read the region layout file.");
    }

    // print
    printConfig();
    std::cout << "\nContents of " << config.RegionLayout << ": " << std::endl;
    printMap();
}
//read region layout
bool Simulation::readRegion(const std::string& path) {
    //open file
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open the region layout file: " << path << std::endl;
        return false;
    }

    std::string line;
    int rowIndex = 0;//variable to track currnet row
    //read each line
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string cellValue;
        std::vector<Cell> row;

        // split line by commas to read each cell value
        while (std::getline(iss, cellValue, ',')) {
            Cell cell;
            char zoneChar = cellValue[0]; // first character to determine the zone type
            //map characters to corresponding zone types
            switch (zoneChar) {
                case 'R': 
                    cell.zone = RESIDENTIAL;
                    break;
                case 'C': 
                    cell.zone = COMMERCIAL; 
                    break;
                case 'I': 
                    cell.zone = INDUSTRIAL; 
                    break;
                case '-': 
                    cell.zone = ROAD; 
                    break;
                case 'T': 
                    cell.zone = POWERLINE; 
                    break;
                case '#': 
                    cell.zone = POWERLINE_OVER_ROAD; 
                    break;
                case 'P': 
                    cell.zone = POWERPLANT; 
                    break;
                case ' ': 
                    cell.zone = EMPTY; 
                    break;
                default: 
                    std::cerr << "Unknown zone type: " << zoneChar << " at row " << rowIndex << std::endl;
                    return false;
            }

            row.push_back(cell); //add cell to current row
        }

        regionMap.push_back(row);//add row to region map
        rowIndex++;
    }

    file.close(); //close file
    return true; //true if successfull
}

//read setting from config file
bool Simulation::readConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open the configuration file: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) { //read lines from file
        std::istringstream iss(line); //create stream
        std::string key, value; 
        std::getline(iss, key, ':'); //read key 
        std::getline(iss, value); //read value

        //set config value based on key
        if (key == "Region Layout") { //set file path for region layout
            config.RegionLayout = value; 
        } else if (key == "Time Limit") { //set timelimit
            config.timeLimit = std::stoi(value); 
        } else if (key == "Refresh Rate") {
            config.refreshRate = std::stoi(value); //set refreshrate
        }
    }

    file.close(); // close config
    return true; // return true when successful
}

/*********************************
PUBLIC MEMBERS
*********************************/

void Simulation::printConfig() const {
    std::cout << "\nConfiguration:" << std::endl;
    std::cout << "- Region Layout File: " << config.RegionLayout << std::endl;
    std::cout << "- Time Limit: " << config.timeLimit << std::endl;
    std::cout << "- Refresh Rate: " << config.refreshRate << std::endl;
}

void Simulation::printMap() const {
    int width = regionMap.empty() ? 0 : regionMap[0].size();

    // Define cell width for consistent formatting
    const int cellWidth = 6;

    // Print the top border
    std::cout << "==" << std::string(width * cellWidth, '=') << "==" << std::endl;

    // Print each row
    for (const auto& row : regionMap) {
        std::cout << "||";
        for (const auto& cell : row) {
            std::ostringstream cellContent;

            if (cell.zone == RESIDENTIAL || cell.zone == COMMERCIAL || cell.zone == INDUSTRIAL) {
                if (cell.population > 0) {
                    cellContent << cell.population;
                    if (cell.pollution > 0) {
                        cellContent << "(" << cell.pollution << ")";
                    }
                } else {
                    cellContent << (cell.zone == RESIDENTIAL ? 'R' :
                                    cell.zone == COMMERCIAL ? 'C' : 'I');
                }
            } else {
                // For other zone types
                switch (cell.zone) {
                    case ROAD:
                        cellContent << '-';
                        break;
                    case POWERLINE:
                        cellContent << 'T';
                        break;
                    case POWERLINE_OVER_ROAD:
                        cellContent << '#';
                        break;
                    case POWERPLANT:
                        cellContent << 'P';
                        break;
                    case EMPTY:
                        cellContent << ' ';
                        break;
                }
                // Append pollution for non-zoned cells
                if (cell.pollution > 0) {
                    cellContent << "(" << cell.pollution << ")";
                }
            }

            // Center the content within the cell width
            std::string cellOutput = cellContent.str();
            int padding = (cellWidth - cellOutput.size()) / 2;
            std::cout << std::string(padding, ' ') << cellOutput << std::string(cellWidth - padding - cellOutput.size(), ' ');
        }
        std::cout << "||" << std::endl;
    }

    // Print the bottom border
    std::cout << "==" << std::string(width * cellWidth, '=') << "==" << std::endl;
}

void Simulation::printResults() const {
    Stats simStats;

    std::cout << "Regional Map: " << std::endl;
    printMap();

    // calc statistics by iterating through map
    for (const auto& row : regionMap) {
        for (const auto& cell : row) {
            // check if any cell is powered
            if (cell.isPowered) {
                simStats.powerOn = true;
            }
            simStats.totalPopulation += cell.population;
            simStats.totalGoods += cell.availableGoods;
            simStats.totalWorkers += cell.availableWorkers;
            simStats.totalPollution += cell.pollution;
        }
    }
    std::cout << "======================\n";
    std::cout << "Power: " << (simStats.powerOn ? "On" : "Off") << "\n";
    std::cout << "Total Population: " << simStats.totalPopulation << "\n";
    std::cout << "Total Goods: " << simStats.totalGoods << "\n";
    std::cout << "Total Workers: " << simStats.totalWorkers << "\n";
    std::cout << "Total Pollution: " << simStats.totalPollution << "\n\n";
}

//**POWER FUNCTIONS**//
// helper function to count adjacent cells with a minimum population
int Simulation::countAdjPop(int x, int y, int minPopulation) const {
    int count = 0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && static_cast<size_t>(nx) < regionMap.size() && ny >= 0 && static_cast<size_t>(ny) < regionMap[0].size()) {
                if (regionMap[nx][ny].population >= minPopulation) {
                    count++;
                }
            }
        }
    }
    return count;
}

// helper function to check if a cell is adjacent to a power line
bool Simulation::hasAdjPower(int x, int y) const {
    std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},  // Up, Down, Left, Right
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1} // Diagonals
    };

    for (const auto& [dx, dy] : directions) {
        int nx = x + dx;
        int ny = y + dy;

        // check if the adjacent cell is within map
        if (nx >= 0 && nx < static_cast<int>(regionMap.size()) && ny >= 0 && ny < static_cast<int>(regionMap[0].size())) {
            // if the adjacent cell is powered power line or power line over road, true
            if ((regionMap[nx][ny].zone == POWERLINE || regionMap[nx][ny].zone == POWERLINE_OVER_ROAD) && regionMap[nx][ny].isPowered) {
                return true;
            }
        }
    }
    // no powered power lines next to this cell
    return false;
}

// function to look for a powerplant(s) on the map
void Simulation::findPowerPlant(std::vector<std::pair<int, int>>& powerPlants) {
    for (int x = 0; x < static_cast<int>(regionMap.size()); x++) {
        for (int y = 0; y < static_cast<int>(regionMap[0].size()); y++) {
            if (regionMap[x][y].zone == POWERPLANT) {
                powerPlants.push_back({x, y});
            }
        }
    }
}

// function that uses BFS to propogate power throughout region
void Simulation::bfsPowerPropagation(int startX, int startY) {
    std::queue<std::pair<int, int>> q;
    q.push({startX, startY});
    regionMap[startX][startY].isPowered = true;
    // std::cout << "Powering cell (" << startX << ", " << startY << ")\n"; // this is for debugging power

    std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},    // Up, Down, Left, Right
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}   // Diagonals
    };

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        for (auto [dx, dy] : directions) {
            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < static_cast<int>(regionMap.size()) && ny >= 0 && ny < static_cast<int>(regionMap[0].size())) {
                // if cell is power line or power line over road and not powered, mark it and continue BFS
                if ((regionMap[nx][ny].zone == POWERLINE || regionMap[nx][ny].zone == POWERLINE_OVER_ROAD) && !regionMap[nx][ny].isPowered) {
                    regionMap[nx][ny].isPowered = true;
                    q.push({nx, ny});
                    // std::cout << "Powering cell (" << nx << ", " << ny << ")\n"; // this is also for debugging power
                }
            }
        }
    }
}

void Simulation::markPowered() {
    for (int x = 0; x < static_cast<int>(regionMap.size()); x++) {
        for (int y = 0; y < static_cast<int>(regionMap[0].size()); y++) {
            if ((regionMap[x][y].zone == POWERLINE || regionMap[x][y].zone == POWERLINE_OVER_ROAD) && regionMap[x][y].isPowered) {

                std::vector<std::pair<int, int>> directions = {
                    {-1, 0}, {1, 0}, {0, -1}, {0, 1},    // Up, Down, Left, Right
                    {-1, -1}, {-1, 1}, {1, -1}, {1, 1}   // Diagonals
                };

                for (auto [dx, dy] : directions) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && static_cast<size_t>(nx) < regionMap.size() && ny >= 0 && static_cast<size_t>(ny) < regionMap[0].size()) {
                        if (regionMap[nx][ny].zone != EMPTY) {
                            regionMap[nx][ny].isPowered = true;
                        }
                    }
                }
            }
        }
    }
}

void Simulation::updatePower() {
    std::vector<std::pair<int, int>> powerPlants;
    findPowerPlant(powerPlants);

    // if no power plants, return (nothing will be powered)
    if (powerPlants.empty()) return;

    // run BFS from each power plant to propagate power through power lines
    for (auto [x, y] : powerPlants) {
        bfsPowerPropagation(x, y);
    }

    // mark cells adjacent to powered power lines as powered
    markPowered();
}

//**SIMULATION HANDLING**//
void Simulation::simulate() {
    int currentTimeStep = 0;
    bool hasChanges = true;

    while (currentTimeStep < config.timeLimit && hasChanges) {
        std::cout << "Timestep " << currentTimeStep + 1 << ":\n";

        hasChanges = false;

        updatePower();          // Step 1: Update power propagation
        residentialGrowth();    // Step 2: Generate workers
        industrialGrowth();     // Step 3: Generate goods and pollution
        commercialGrowth();     // Step 4: Consume goods and workers
        spreadPollution();      // Step 5: Spread pollution from Industrial zones

        hasChanges = detectChanges(); // Step 6: Detect changes

        if (currentTimeStep % config.refreshRate == 0 || !hasChanges) {
            printResults();
        }

        currentTimeStep++;
    }

    std::cout << "Simulation complete.\nFinal state:\n";
    printResults();
}

// Helper function to detect changes in the map 
bool Simulation::detectChanges() {
    static std::vector<std::vector<int>> previousPopulation(regionMap.size(),
                                                            std::vector<int>(regionMap[0].size(), 0));
    bool hasChanged = false;

    for (size_t x = 0; x < regionMap.size(); x++) {
        for (size_t y = 0; y < regionMap[x].size(); y++) {
            // Check if population, goods, or pollution has changed
            const Cell& cell = regionMap[x][y];
            if (cell.population != previousPopulation[x][y]) {
                hasChanged = true;
                previousPopulation[x][y] = cell.population;
            }
        }
    }

    return hasChanged;
}

//**GROWTH FUNCTIONS**//
void Simulation::residentialGrowth() {
    std::vector<std::tuple<int, int, int>> growthCandidates;  // Store (x, y, priority)

    // First pass: Find all residential cells eligible for growth
    for (int x = 0; x < static_cast<int>(regionMap.size()); x++) {
        for (int y = 0; y < static_cast<int>(regionMap[0].size()); y++) {
            Cell& cell = regionMap[x][y];
            if (cell.zone == RESIDENTIAL) {
                bool eligibleForGrowth = false;
                int adjCount = 0;

                // Determine eligibility based on population level and adjacency requirements
                switch (cell.population) {
                    case 0:
                        eligibleForGrowth = hasAdjPower(x, y) || countAdjPop(x, y, 1) > 0;
                        break;
                    case 1:
                        adjCount = countAdjPop(x, y, 1);
                        eligibleForGrowth = (adjCount >= 2);
                        break;
                    case 2:
                        adjCount = countAdjPop(x, y, 2);
                        eligibleForGrowth = (adjCount >= 4);
                        break;
                    case 3:
                        adjCount = countAdjPop(x, y, 3);
                        eligibleForGrowth = (adjCount >= 6);
                        break;
                    case 4:
                        adjCount = countAdjPop(x, y, 4);
                        eligibleForGrowth = (adjCount >= 8);
                        break;
                }

                // If cell is eligible for growth, add it to candidates
                if (eligibleForGrowth) {
                    growthCandidates.emplace_back(x, y, cell.population);
                }
            }
        }
    }

    // Sort candidates based on growth rules: higher population first, then smaller coordinates
    std::sort(growthCandidates.begin(), growthCandidates.end(), [](const auto& a, const auto& b) {
        return std::tie(std::get<2>(b), std::get<1>(a), std::get<0>(a)) < std::tie(std::get<2>(a), std::get<1>(b), std::get<0>(b));
    });

    // Apply growth to eligible cells
    for (const auto& [x, y, _] : growthCandidates) {
        Cell& cell = regionMap[x][y];
        cell.population++;

        // Generate workers based on new population
        cell.availableWorkers = cell.population;  // Example: 1 worker per population unit
    }
}

void Simulation::industrialGrowth() {
    std::vector<std::tuple<int, int, int>> growthCandidates;

    // First pass: Identify all eligible industrial cells for growth
    for (int x = 0; x < static_cast<int>(regionMap.size()); x++) {
        for (int y = 0; y < static_cast<int>(regionMap[0].size()); y++) {
            Cell& cell = regionMap[x][y];
            if (cell.zone == INDUSTRIAL) {
                bool eligible = false;

                // Determine growth eligibility based on population level
                switch (cell.population) {
                    case 0:
                        eligible = (hasAdjPower(x, y) || countAdjPop(x, y, 1) > 0) &&
                                   countAvailableWorkers() >= 2;
                        break;
                    case 1:
                        eligible = countAdjPop(x, y, 1) >= 2 && countAvailableWorkers() >= 2;
                        break;
                    case 2:
                        eligible = countAdjPop(x, y, 2) >= 4 && countAvailableWorkers() >= 2;
                        break;
                }

                // Add eligible cells to growth candidates
                if (eligible) {
                    growthCandidates.emplace_back(x, y, cell.population);
                }
            }
        }
    }

    // Sort candidates by growth priority: larger population first, then adjacency count
    std::sort(growthCandidates.begin(), growthCandidates.end(), [](const auto& a, const auto& b) {
        return std::tie(std::get<2>(b), std::get<1>(a), std::get<0>(a)) <
               std::tie(std::get<2>(a), std::get<1>(b), std::get<0>(b));
    });

    // Second pass: Apply growth to the eligible cells
    for (const auto& [x, y, _] : growthCandidates) {
        Cell& cell = regionMap[x][y];
        if (countAvailableWorkers() >= 2) { // Ensure enough workers are available
            cell.population++; // Increment population
            assignWorkerToJob(); // Deduct 2 workers
            cell.availableGoods += cell.population; // Produce goods
        }
    }
}

void Simulation::commercialGrowth() {
    std::vector<std::tuple<int, int, int>> growthCandidates;

    for (int x = 0; x < static_cast<int>(regionMap.size()); x++) {
        for (int y = 0; y < static_cast<int>(regionMap[0].size()); y++) {
            Cell& cell = regionMap[x][y];
            if (cell.zone == COMMERCIAL) {
                bool eligible = false;

                switch (cell.population) {
                    case 0:
                        eligible = hasAdjPower(x, y) && countAvailableWorkers() >= 1 && countAvailableGoods() >= 1;
                        break;
                    case 1:
                        eligible = countAdjPop(x, y, 1) >= 2 && countAvailableWorkers() >= 1 && countAvailableGoods() >= 1;
                        break;
                    case 2:
                        eligible = countAdjPop(x, y, 2) >= 4 && countAvailableWorkers() >= 1 && countAvailableGoods() >= 1;
                        break;
                }

                if (eligible) {
                    growthCandidates.emplace_back(x, y, cell.population);
                }
            }
        }
    }

    // Sort and grow Commercial zones
    std::sort(growthCandidates.begin(), growthCandidates.end(), [](const auto& a, const auto& b) {
        return std::tie(std::get<2>(b), std::get<1>(a), std::get<0>(a)) <
               std::tie(std::get<2>(a), std::get<1>(b), std::get<0>(b));
    });

    for (const auto& [x, y, _] : growthCandidates) {
        Cell& cell = regionMap[x][y];
        cell.population++;
        assignWorkerToJob();  // Deduct 1 worker
        assignGoodToCell();   // Deduct 1 good
    }
}

void Simulation::spreadPollution() {
    std::queue<std::tuple<int, int, int>> pollutionQueue;

    // Initialize the queue with all industrial cells
    for (int x = 0; x < static_cast<int>(regionMap.size()); x++) {
        for (int y = 0; y < static_cast<int>(regionMap[0].size()); y++) {
            Cell& cell = regionMap[x][y];

            if (cell.zone == INDUSTRIAL && cell.population > 0) {
                pollutionQueue.emplace(x, y, cell.population); // (x, y, pollution level)
            }
        }
    }

    // Directions for adjacency (including diagonals)
    std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},    // Up, Down, Left, Right
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}  // Diagonals
    };

    // BFS to spread pollution
    while (!pollutionQueue.empty()) {
        auto [x, y, pollutionLevel] = pollutionQueue.front();
        pollutionQueue.pop();

        for (const auto& [dx, dy] : directions) {
            int nx = x + dx;
            int ny = y + dy;

            // Check bounds
            if (nx >= 0 && nx < static_cast<int>(regionMap.size()) &&
                ny >= 0 && ny < static_cast<int>(regionMap[0].size())) {
                Cell& adjacentCell = regionMap[nx][ny];

                // Spread pollution if decayed value is greater than current pollution
                int decayedPollution = pollutionLevel - 1;
                if (decayedPollution > 0 && adjacentCell.pollution < decayedPollution) {
                    adjacentCell.pollution = decayedPollution;
                    pollutionQueue.emplace(nx, ny, decayedPollution); // Continue spreading
                }
            }
        }
    }
}

void Simulation::produceGoods() {
    for (int x = 0; x < static_cast<int>(regionMap.size()); x++) {
        for (int y = 0; y < static_cast<int>(regionMap[0].size()); y++) {
            Cell& cell = regionMap[x][y];

            if (cell.zone == INDUSTRIAL && cell.population > 0) {
                // Produce goods based on population
                cell.availableGoods += cell.population;
            }
        }
    }
}

void Simulation::assignGoodToCell() {
    for (auto& row : regionMap) {
        for (auto& cell : row) {
            if (cell.zone == INDUSTRIAL && cell.availableGoods > 0) {
                cell.availableGoods--; // Deduct 1 good
                return;
            }
        }
    }
}

int Simulation::countAvailableGoods() const {
    int totalGoods = 0;
    for (const auto& row : regionMap) {
        for (const auto& cell : row) {
            if (cell.zone == INDUSTRIAL) {
                totalGoods += cell.availableGoods;
            }
        }
    }
    return totalGoods;
}

int Simulation::countAvailableWorkers() const {
    int totalWorkers = 0;
    for (const auto& row : regionMap) {
        for (const auto& cell : row) {
            totalWorkers += cell.availableWorkers;
        }
    }
    return totalWorkers;
}

void Simulation::assignWorkerToJob() {
    for (auto& row : regionMap) {
        for (auto& cell : row) {
            if (cell.zone == RESIDENTIAL && cell.availableWorkers >= 2) {
                cell.availableWorkers -= 2;  // Deduct 2 workers for industrial jobs
                return;
            }
        }
    }
}
