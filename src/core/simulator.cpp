#include "simulator.h" // declares the file function
#include "physics.h" // used for hasItem,RemoveItem etc(member 1)
#include "switches.h" 
#include "logger.h"// used to save file functions
#include "viewer_iface.h" //used for screen dawing functions
#include "level.h" // (symbol to word)
#include "util.h" //
#include <iostream>
// Removes one symbol from a room's list and moves the rest left (no gaps).
//static= only this file can use this function
// char roomlist is the item symbols available in the room
// int & count is to count the items in the room and & is the pass by reference
//char symbol to remove a particular item like '*'
static bool removeFromRoom(char roomList[MAX_ENTITIES_PER_ROOM], int& count, char symbol) {
    int position = -1; //initialised from -1 because the index of array starts from 0
    for (int i = 0; i < count; i++) {
        if (roomList[i] == symbol) {//check if its the item we want to remove
            position = i;
            break;// because there is no further need to keep searching the item
        }
    }
    if (position == -1) {
        return false;   // not in this room
    }

    for (int i = position; i < count - 1; i++) {
        roomList[i] = roomList[i + 1]; // after removing the item we want to move each item to left index
    }
    count--;
    roomList[count] = ENT_NONE; // marking unused last position as empty
    return true; //removing item was successful
}

// True for things that are fixed in place and can't be picked up.
static bool isFixedThing(char symbol) {
    return symbol == ENT_CLUE_4 || symbol == ENT_CLUE_2 || symbol == ENT_CLUE_7 ||
           symbol == ENT_CAT || symbol == ENT_PASSENGER ||
           symbol == ENT_CONTROL_PANEL || symbol == ENT_CABINET_TRAIN_KEY;//return true if the symbol matches any fixed object
}

// Splits "move F3-R2" into word = "move" and argument = "F3-R2".
//char* is a pointer to character data
//const means the function shouldnt change the original input through the line
static void splitCommand(const char* line, char word[MAX_LINE_LEN], char argument[MAX_LINE_LEN]) {
    int i = 0;//keep the track of position in line
    int w = 0;// keep the track of position in word
    while (line[i] != '\0' && line[i] != ' ' && w < MAX_LINE_LEN - 1) { //if it didnt reach the end
        // if it didnt reach the space
        // if there is another room to store the char safely
        word[w] = line[i];
        w++;
        i++;
    }
    word[w] = '\0'; // always present in char array as it tells where the string ends

    while (line[i] == ' ') {   // skip the spaces in the middle
        i++;
    }

    int a = 0;
    while (line[i] != '\0' && a < MAX_LINE_LEN - 1) { // extracts everything remaining after the spaces
        argument[a] = line[i];
        a++;
        i++;
    }
    argument[a] = '\0';
    trimWhitespace(argument); //removes unecessary space (" n "->"n")
}

// Adds energy and time to the log. If the arrays are full, makes them
// twice as big: new bigger arrays, copy, delete the old ones.
// int*& means this function can change where main's pointer points.
static void addToLog(int*& energyLog, int*& timeLog, int& count, int& capacity,
                     int energy, int time) {

    // If arrays are full, increase their capacity
    if (count == capacity) {
        // Double the old capacity
        int newCapacity = capacity * 2;
        // Create bigger arrays
        int* biggerEnergy = new int[newCapacity];
        int* biggerTime = new int[newCapacity];
        // Copy old data into the bigger arrays
        for (int i = 0; i < count; i++) {
            biggerEnergy[i] = energyLog[i];
            biggerTime[i] = timeLog[i];
        }
        // Release memory used by old arrays
        delete[] energyLog;
        delete[] timeLog;
        // Make pointers point to the new arrays
        energyLog = biggerEnergy;
        timeLog = biggerTime;
        // Update capacity
        capacity = newCapacity;
    }
    // Add the new energy and time values
    energyLog[count] = energy;
    timeLog[count] = time;
    // Increase the number of stored entries
    count++;
}
// Displays all available game commands and their descriptions.
// No parameters are needed because the instructions are fixed.
static void printHelp() {
    std::cout << "\n COMMANDS\n";
    std::cout << "  move F3-R2    go to a room next to you\n";
    std::cout << "  inspect       search this room for clues\n";
    std::cout << "  collect <     pick up an item (type its symbol)\n";
    std::cout << "  use %         use an item from your bag (type its symbol)\n";
    std::cout << "  unlock 427    open the cabinet in F5-R1\n";
    std::cout << "  map           show the building\n";
    std::cout << "  goals         show your goals\n";
    std::cout << "  save          save the game\n";
    std::cout << "  help          show this list\n";
    std::cout << "  quit          save and exit\n";
}
// The game loop
// Function: Main game loop that controls the entire simulation.
// int& means changes made to these variables also affect the original variables.
// char arrays store inventory and room information.
// bool array stores whether each objective is completed.
void runSimulation(int& playerFloor, int& playerRoom,
                   int& energy, int& time, int& score,
                   char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                   bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft,
                   char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                   char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                   int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]) {
    // Dynamic memory: a log of energy and time after every action
    // Dynamic memory: stores energy and time after every action.
    int capacity = 32; // Maximum number of entries initially.
    int logCount = 0; // Number of entries currently stored.
    int* energyLog = new int[capacity]; // Dynamically creates an energy array.
    int* timeLog = new int[capacity]; // Dynamically creates a time array.
     // Store the starting energy and time before the game begins.
    addToLog(energyLog, timeLog, logCount, capacity, energy, time);   // step 0 = start
    char line[MAX_LINE_LEN]; // Stores the complete command entered by the player.
    char word[MAX_LINE_LEN]; // Stores the command word.
    char argument[MAX_LINE_LEN]; // Stores the command argument.
    bool gameOver = false; // Initially, the game is not over.
    printHelp(); //display all avaialble commands
   // Repeat the game until one of the ending conditions is met.
    while (!gameOver) {
        // Display the player's current status and room information.
        renderDashboard(playerFloor, playerRoom, energy, time, score,
                        inventory, inventoryCount, emergencyTripsLeft);
        renderRoomDetails(playerFloor, playerRoom, roomEntities,
                          entityCounts[playerFloor][playerRoom]);
        std::cout << "> ";
        // Read the command entered by the player.
        if (!std::cin.getline(line, MAX_LINE_LEN)) {
            // If there is no more input, save the game and exit.
            std::cout << "\nNo more input. Saving and exiting.\n";
            saveGameStateToFile("out/savegame.txt", playerFloor, playerRoom, energy, time, score,
                                inventory, inventoryCount, objectives, emergencyTripsLeft,
                                roomEntities, entityCounts);
            break; // Exit the game loop.
        }
       trimWhitespace(line); // Remove unnecessary spaces from the command.
        if (line[0] == '\0') {
            continue; // If the command is empty, start the loop again.
        }
        std::cout << line << "\n"; // Display the command entered by the player.
        splitCommand(line, word, argument); // Separate the command into word and argument.
        // Check whether Lift 2 is working.
        bool lift2Working = objectives[OBJ_FUSE_INSTALLED] && objectives[OBJ_LIFT_PASS];
        bool actionDone = false; // Tracks whether the player performed a valid action.
        // Check which command the player entered.
        if (stringEquals(word, "move")) {
            int targetFloor = 0; // Stores the destination floor.
            int targetRoom = 0; // Stores the destination room.
            // Check whether the entered room ID is valid.
            if (!parseRoomId(argument, targetFloor, targetRoom)) {
                std::cout << "Please type a room like this: move F3-R2\n";
            } else {
                // Handle the player's movement to another room.
                actionDone = handleMoveCommand(targetFloor, targetRoom, playerFloor, playerRoom,
                                               energy, time, horizontalConn,
                                               inventory, inventoryCount,
                                               emergencyTripsLeft, lift2Working);
            }
        } else if (stringEquals(word, "inspect")) {
            // Search the current room for clues and other objects.
            actionDone = handleInspectCommand(playerFloor, playerRoom, energy, time,
                                              roomEntities,
                                              entityCounts[playerFloor][playerRoom],
                                              objectives, score);
        } else if (stringEquals(word, "collect")) {
            // Check whether the player entered an item symbol.
            if (argument[0] == '\0') {
                std::cout << "Collect what? Example: collect <\n";
            } else {
                char symbol = argument[0]; // Store the symbol of the item to collect.
                // Handle collecting the selected item.
                actionDone = handleCollectCommand(symbol, playerFloor, playerRoom,
                                                  energy, time, score, roomEntities,
                                                  entityCounts[playerFloor][playerRoom],
                                                  inventory, inventoryCount, objectives);
                // Give the player three emergency trips if they collect the card.
                if (actionDone && symbol == ENT_EMERGENCY_CARD) {
                    emergencyTripsLeft = 3;
                    std::cout << "You can now use Lift 2 three times in an emergency.\n";
                }
            }
        } else if (stringEquals(word, "use")) {
            // Check whether the player entered an item symbol.
            if (argument[0] == '\0') {
                std::cout << "Use what? Example: use %\n";
            } else {
                char symbol = argument[0]; // Store the selected item symbol.
                // Use the selected item from the player's inventory.
                actionDone = handleUseCommand(symbol, playerFloor, playerRoom,
                                              energy, time, score,
                                              inventory, inventoryCount, objectives);
                // Remove the passenger from the room after using the Medical Kit.
                if (actionDone && symbol == ENT_MEDICAL_KIT) {
                    removeFromRoom(roomEntities[playerFloor][playerRoom],
                                   entityCounts[playerFloor][playerRoom], ENT_PASSENGER);
                }
                // Remove the cat from the room after using the Cat Carrier.
                if (actionDone && symbol == ENT_CAT_CARRIER) {
                    removeFromRoom(roomEntities[playerFloor][playerRoom],
                                   entityCounts[playerFloor][playerRoom], ENT_CAT);
                }
            }

        } else if (stringEquals(word, "unlock")) {
            // Try to unlock the cabinet using the entered code.
            actionDone = handleUnlockCabinetCommand(argument, playerFloor, playerRoom,
                                                    objectives, score, roomEntities,
                                                    entityCounts[playerFloor][playerRoom]);
            if (actionDone) {
                // Add the Train Key to the player's inventory.
                addItem(inventory, inventoryCount, ENT_CABINET_TRAIN_KEY);
                // Reduce energy and time for using the item.
                applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
            }
        } else if (stringEquals(word, "map")) {
            renderAsciiBuilding(playerFloor, playerRoom); // Display the building map.
        } else if (stringEquals(word, "goals")) {
            renderObjectivesChecklist(objectives); // Display the completed and remaining goals.
        } else if (stringEquals(word, "save")) {
            // Save the current game information into a file.
            if (saveGameStateToFile("out/savegame.txt", playerFloor, playerRoom, energy, time, score,
                                    inventory, inventoryCount, objectives, emergencyTripsLeft,
                                    roomEntities, entityCounts)) {
                std::cout << "Game saved to out/savegame.txt\n";
            }
        } else if (stringEquals(word, "help")) {
            printHelp(); // Display the list of available commands.
        } else if (stringEquals(word, "quit")) {
            // Save the game before exiting.
            saveGameStateToFile("out/savegame.txt", playerFloor, playerRoom, energy, time, score,
                                inventory, inventoryCount, objectives, emergencyTripsLeft,
                                roomEntities, entityCounts);
            std::cout << "Game saved. Goodbye!\n";
            break; // Exit the game loop.
        } else {
            // Display a message if the player enters an unknown command.
            std::cout << "Unknown command. Type help to see the commands.\n";
        }
        // Only successful actions are added to the resource log.
        if (actionDone) {
            addToLog(energyLog, timeLog, logCount, capacity, energy, time);
            // Check whether the game should end.
            gameOver = checkEndConditions(energy, time, objectives);
        }
    }
    // If the game ends, check whether the player won or lost.
    if (gameOver) {
        bool victory = objectives[OBJ_MISSION_WON]; // Check if the mission was completed.
        if (!victory) {
            objectives[OBJ_MISSION_FAILED] = true; // Mark the mission as failed.
        }
        renderObjectivesChecklist(objectives); // Display the final objectives.
        if (victory) {
            printHeader("MISSION COMPLETE - YOU CAUGHT THE LAST TRAIN!");
        } else {
            printHeader("MISSION FAILED - YOU RAN OUT OF ENERGY OR TIME");
        }
        std::cout << " Final score: " << score << "/520\n";
        // Generate the final game report and resource log.
        writeGameSummaryReport("out/game_summary.txt", score, energy, time, objectives, victory);
        writeResourceLog("out/resource_log.txt", energyLog, timeLog, logCount);
        std::cout << " Report written to out/game_summary.txt\n";
    }
    // Release dynamically allocated memory to prevent memory leaks.
    delete[] energyLog;
    delete[] timeLog;
    // Set pointers to nullptr after deleting the arrays.
    energyLog = nullptr;
    timeLog = nullptr;
}
// inspect: shows everything in the room and finds clue digits
bool handleInspectCommand(int currentFloor, int currentRoom,
                          int& energy, int& time,
                          const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int entityCount,
                          bool objectives[NUM_OBJECTIVES], int& score) {
    applyResourceDelta(energy, time, COST_INSPECT_ENERGY, COST_INSPECT_TIME);
    std::cout << "You search " << getRoomName(currentFloor, currentRoom)
              << " carefully. (-1 Energy, -1 Time)\n";

    if (entityCount == 0) {
        std::cout << "There is nothing here.\n";
        return true;
    }
    for (int i = 0; i < entityCount; i++) {
        char symbol = roomEntities[currentFloor][currentRoom][i];
        std::cout << "  - " << getEntityName(symbol) << " (" << symbol << ")\n";
        if (symbol == ENT_CLUE_4 && !objectives[OBJ_CLUE_4]) {
            objectives[OBJ_CLUE_4] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            std::cout << "  You found clue digit 4! (+10)\n";
        }
        if (symbol == ENT_CLUE_2 && !objectives[OBJ_CLUE_2]) {
            objectives[OBJ_CLUE_2] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            std::cout << "  You found clue digit 2! (+10)\n";
        }
        if (symbol == ENT_CLUE_7 && !objectives[OBJ_CLUE_7]) {
            objectives[OBJ_CLUE_7] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            std::cout << "  You found clue digit 7! (+10)\n";
        }
    }
    return true;
}
// collect: picks an item up from the room
bool handleCollectCommand(char itemSymbol, int currentFloor, int currentRoom,
                          int& energy, int& time, int& score,
                          char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int& entityCount,
                          char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                          bool objectives[NUM_OBJECTIVES]) {
    // 1. Is the item in this room?
    bool found = false;
    for (int i = 0; i < entityCount; i++) {
        if (roomEntities[currentFloor][currentRoom][i] == itemSymbol) {
            found = true;
        }
    }
    if (!found) {
        std::cout << "There is no '" << itemSymbol << "' in this room.\n";
        return false;
    }
    // 2. Some things can't be picked up
    if (isFixedThing(itemSymbol)) {
        std::cout << "You can't pick up the " << getEntityName(itemSymbol) << ".\n";
        return false;
    }
    // 3. Items that work straight away (not kept in the bag)
    if (itemSymbol == ENT_ENERGY_PACK) {
        applyResourceDelta(energy, time, COST_ITEM_PICKUP_ENERGY, COST_ITEM_PICKUP_TIME);
        applyResourceDelta(energy, time, BONUS_ENERGY_PACK, 0);
        std::cout << "You use the Energy Pack. (+25 Energy)\n";
    } else if (itemSymbol == ENT_EMERGENCY_TOOLKIT) {
        applyResourceDelta(energy, time, COST_ITEM_PICKUP_ENERGY, COST_ITEM_PICKUP_TIME);
        applyResourceDelta(energy, time, BONUS_EMERGENCY_TOOLKIT, 0);
        std::cout << "You use the Emergency Toolkit. (+15 Energy)\n";
    } else if (itemSymbol == ENT_OBSERVATION_TOKEN) {
        applyResourceDelta(energy, time, COST_ITEM_PICKUP_ENERGY, COST_ITEM_PICKUP_TIME);
        applyResourceDelta(energy, time, 0, BONUS_OBSERVATION_TOKEN);
        std::cout << "You use the Observation Token. (+15 Time)\n";
    } else {
        // 4. Normal items go into the bag
        if (!addItem(inventory, inventoryCount, itemSymbol)) {
            std::cout << "Your bag is full.\n";
            return false;
        }
        applyResourceDelta(energy, time, COST_ITEM_PICKUP_ENERGY, COST_ITEM_PICKUP_TIME);
        std::cout << "You put the " << getEntityName(itemSymbol) << " in your bag. (-1 Energy, -1 Time)\n";
        if (itemSymbol == ENT_MASTER_KEY) {
            objectives[OBJ_MASTER_KEY] = true;
            applyScoreDelta(score, SCORE_KEY_COLLECT);
            std::cout << "This key opens the locked door on Floor 3. (+10)\n";
        } else if (itemSymbol == ENT_LIFT_PASS) {
            objectives[OBJ_LIFT_PASS] = true;
            applyScoreDelta(score, SCORE_LIFT_PASS);
            std::cout << "Lift Pass collected. (+20)\n";
            if (objectives[OBJ_FUSE_INSTALLED]) {
                objectives[OBJ_LIFT2_ACTIVE] = true;
                std::cout << "Lift 2 is now working!\n";
            }
        } else if (itemSymbol == ENT_FUSE) {
            objectives[OBJ_FUSE_COLLECTED] = true;
            std::cout << "Take the Fuse to the Control Panel in F3-R4.\n";
        } else if (itemSymbol == ENT_CAT_CARRIER) {
            objectives[OBJ_CAT_CARRIER] = true;
            std::cout << "Now you can secure the cat in F3-R3.\n";
        }
    }
    // 5. Take it out of the room
    removeFromRoom(roomEntities[currentFloor][currentRoom], entityCount, itemSymbol);
    return true;
}
// use: uses an item from the bag in the right room
bool handleUseCommand(char itemSymbol, int currentFloor, int currentRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES]) {
    if (!hasItem(inventory, inventoryCount, itemSymbol)) {
        std::cout << "You don't have that in your bag.\n";
        return false;
    }
    // Fuse -> Control Panel in F3-R4 (floor 2, room 3)
    if (itemSymbol == ENT_FUSE) {
        if (currentFloor != 2 || currentRoom != 3) {
            std::cout << "The Control Panel is in F3-R4.\n";
            return false;
        }
        if (!installFuseControlPanel(inventory, inventoryCount, objectives, score)) {
            std::cout << "The Fuse could not be installed.\n";
            return false;
        }
        applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
        std::cout << "Fuse installed in the Control Panel. (+20)\n";
        if (objectives[OBJ_LIFT2_ACTIVE]) {
            std::cout << "Lift 2 is now working!\n";
        } else {
            std::cout << "Lift 2 also needs the Lift Pass.\n";
        }
        return true;
    }
    // Medical Kit -> Passenger in F4-R1 (floor 3, room 0)
    if (itemSymbol == ENT_MEDICAL_KIT) {
        if (currentFloor != 3 || currentRoom != 0) {
            std::cout << "The injured passenger is in F4-R1.\n";
            return false;
        }
        removeItem(inventory, inventoryCount, ENT_MEDICAL_KIT);
        objectives[OBJ_PASSENGER_RESCUED] = true;
        applyScoreDelta(score, SCORE_RESCUE_PASSENGER);
        applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
        std::cout << "You treat the passenger. Passenger rescued! (+100)\n";
        return true;
    }
    // Cat Carrier -> Cat in F3-R3 (floor 2, room 2)
    if (itemSymbol == ENT_CAT_CARRIER) {
        if (currentFloor != 2 || currentRoom != 2) {
            std::cout << "The cat is in F3-R3.\n";
            return false;
        }
        removeItem(inventory, inventoryCount, ENT_CAT_CARRIER);
        objectives[OBJ_CAT_SECURED] = true;
        applyScoreDelta(score, SCORE_SECURE_CAT);
        applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
        std::cout << "The cat is safe in the carrier. Cat secured! (+100)\n";
        return true;
    }
    // Train Key -> Train in F5-R2 (floor 4, room 1)
    if (itemSymbol == ENT_CABINET_TRAIN_KEY) {
        if (currentFloor != 4 || currentRoom != 1) {
            std::cout << "The train is at the Train Boarding Platform, F5-R2.\n";
            return false;
        }
        if (!objectives[OBJ_PASSENGER_RESCUED] || !objectives[OBJ_CAT_SECURED]) {
            std::cout << "You can't leave yet. Rescue the passenger and the cat first.\n";
            return false;
        }
        objectives[OBJ_TRAIN_PREPARED] = true;
        applyScoreDelta(score, SCORE_PREPARE_TRAIN);
        objectives[OBJ_MISSION_WON] = true;
        applyScoreDelta(score, SCORE_MISSION_VICTORY);
        applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
        std::cout << "The train is ready and everyone is on board! (+100, +100 bonus)\n";
        return true;
    }
    // Master Key works by itself when moving through the locked door
    if (itemSymbol == ENT_MASTER_KEY) {
        std::cout << "The Master Key opens the locked door when you move through it.\n";
        return false;
    }
    std::cout << "Nothing happens.\n";
    return false;
}
// unlock: opens the cabinet in F5-R1 with the code
bool handleUnlockCabinetCommand(const char* code, int currentFloor, int currentRoom,
                                bool objectives[NUM_OBJECTIVES], int& score,
                                char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                                int& entityCount) {
    if (currentFloor != 4 || currentRoom != 0) {
        std::cout << "The cabinet is in the Security Room, F5-R1.\n";
        return false;
    }
    if (objectives[OBJ_CABINET_UNLOCKED]) {
        std::cout << "The cabinet is already open.\n";
        return false;
    }
    if (!objectives[OBJ_CLUE_4] || !objectives[OBJ_CLUE_2] || !objectives[OBJ_CLUE_7]) {
        std::cout << "You don't know the whole code yet. Find all three clue digits first.\n";
        return false;
    }
    if (!verifyCabinetCode(code, objectives)) {
        std::cout << "Wrong code. The cabinet stays locked.\n";
        return false;
    }
    objectives[OBJ_CABINET_UNLOCKED] = true;
    applyScoreDelta(score, SCORE_CABINET_UNLOCK);
    objectives[OBJ_TRAIN_KEY] = true;
    applyScoreDelta(score, SCORE_TRAIN_KEY);
    removeFromRoom(roomEntities[currentFloor][currentRoom], entityCount, ENT_CABINET_TRAIN_KEY);
    std::cout << "Click! The cabinet opens. You take the Train Key. (+30, +10)\n";
    return true;
}
// The game ends if energy or time is 0, or if the mission is won
bool checkEndConditions(int energy, int time, const bool objectives[NUM_OBJECTIVES]) {
    if (energy <= 0 || time <= 0) {
        return true;
    }
    if (objectives[OBJ_MISSION_WON]) {
        return true;
    }
    return false;
}
