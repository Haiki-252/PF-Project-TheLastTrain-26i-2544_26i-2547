#include "simulator.h"
#include "level.h"
#include "logger.h"
#include "viewer_iface.h"
#include "util.h"
#include <iostream>

int main() {
    // Player State Primitives
    int playerFloor = 1;
    int playerRoom = 1;
    int energy = INITIAL_ENERGY;
    int time = INITIAL_TIME;
    int score = INITIAL_SCORE;
    
    char inventory[MAX_INVENTORY_CAPACITY];
    int inventoryCount = 0;
    int emergencyTripsLeft = 0;

    // Objective State Booleans
    bool objectives[NUM_OBJECTIVES] = { false };

    // Building Connectivity & Entity Parallel Matrices
    char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1];
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM];
    int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR] = { 0 };

    const char* mapFile = "data/building.txt";
    const char* initStateFile = "data/initial-state.txt";

    if (!loadBuildingMap(mapFile, horizontalConn, roomEntities, entityCounts)) {
        std::cerr << "Error: Could not load " << mapFile << "\n";
        // std::cerr is specifically meant for error messages and prints immediately without waiting in a buffer.
        return 1;
    }

    if (!loadInitialState(initStateFile, playerFloor, playerRoom, energy, time, score,
                          inventory, inventoryCount, objectives, emergencyTripsLeft)) {
        std::cerr << "Error: Could not load " << initStateFile << "\n";
        return 1;
    }

    std::cout << "===========================================\n";
    std::cout << " THE LAST TRAIN - SIMULATION INITIALIZED   \n";
    std::cout << "===========================================\n";
    //Ask the player for a valid starting room (e.g F1-R1)
    char startInput[MAX_LINE_LEN];
    while (true)
    {
        std::cout << "Enter starting room (e.g. F1-R1) or press Enter for default F"<< (playerFloor + 1) << "-R" << (playerRoom + 1) << ": ";
        if (!std::cin.getline(startInput, MAX_LINE_LEN))
        {
            break; // Valid starting room entered
        }
        trimWhitespace(startInput);
        if (startInput[0] == '\0')
        {
            if (isValidStartingRoom(playerFloor, playerRoom, horizontalConn))
            {
                break; // Default starting room is valid
            }
            else
            {
                std::cout << "Default starting room F" << (playerFloor + 1) << "-R" << (playerRoom + 1) << " is invalid. Please enter a valid starting room.\n";
                continue;
            }
        }
        int chosenFloor = 0;
        int chosenRoom = 0;
        if (!parseRoomId(startInput, chosenFloor, chosenRoom)) 
        {
            std::cout << "Invalid starting room. Please enter a valid room from F1-R1 to F5-R4.\n";
            continue;
        }
        if (!isValidStartingRoom(chosenFloor, chosenRoom, horizontalConn)) {
            std::cout << "F" << (chosenFloor + 1) << "-R" << (chosenRoom + 1)<< " cannot reach all mission goals. Try another room.\n";
            continue;
        }
        playerFloor = chosenFloor;
        playerRoom = chosenRoom;
        break;
    }
    runSimulation(playerFloor, playerRoom, energy, time, score,
                  inventory, inventoryCount, objectives, emergencyTripsLeft,
                  horizontalConn, roomEntities, entityCounts);

    return 0;
}
