#include "viewer_iface.h" // Contains declarations of display functionss
#include "level.h"        // Contains floor, room and entity information
#include "util.h"         // Contains utility functions like clampInt()
#include <iostream>       // Allows us to use cout for displaying output

// Stores the names of all 20 rooms
// NUM_FLOORS = rows and ROOMS_PER_FLOOR = columns
const char* roomNames[NUM_FLOORS][ROOMS_PER_FLOOR] = {
    { "Entrance Lobby",   "Reception",          "Waiting Area",          "Utility Room" },
    { "Lounge",           "Archive",            "Lab",                   "Meeting Room" },
    { "Stairwell Annex",  "Security Vestibule", "Animal Room",           "Control Room" },
    { "First Aid Station","Medical Room",       "Maintenance Corridor",  "Office" },
    { "Security Room",    "Train Boarding Platform", "Storage Room",     "Observation Deck" }
};
// floor number and room number
// const char* means the function returns a pointer to a constant character string
const char* getRoomName(int floor, int room) {

    // Check if the floor or room number is outside the valid range
    if (floor < 0 || floor >= NUM_FLOORS || room < 0 || room >= ROOMS_PER_FLOOR) {
        return "Unknown Room"; // Return this if the numbers are invalid
    }

    // Return the room name from the 2D array
    return roomNames[floor][room];
}

// Draws a visual bar showing energy or time
// static means this function can only be used inside this file
// int value = the amount of energy or time
static void printBar(int value) {

    // Keep the value between 0 and 100, then divide by 10
    // This tells us how many '#' symbols should be displayed
    int filled = clampInt(value, 0, 100) / 10;

    std::cout << "[";

    // Loop runs 10 times because the bar contains 10 positions
    for (int i = 0; i < 10; i++) {

        if (i < filled) {
            std::cout << "#"; // Display a filled position
        } else {
            std::cout << "-"; // Display an empty position
        }
    }

    std::cout << "]"; // Close the bar
}

//  Displays the player's complete status dashboard
// int parameters store the player's current information
// const inventory means this function cannot change the inventory
// int emergencyTripsLeft stores the remaining emergency lift trips
void renderDashboard(int playerFloor, int playerRoom, int energy, int time, int score,
                     const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                     int emergencyTripsLeft) {

    std::cout << "\n------------------------------------------\n";

    // Display the player's current location and room name
    // +1 is used because array indexes start from 0, but the game shows numbers from 1
    std::cout << " Location : F" << playerFloor + 1 << "-R" << playerRoom + 1
              << " (" << getRoomName(playerFloor, playerRoom) << ")\n";

    // Display the energy bar and its numerical value
    std::cout << " Energy   : ";
    printBar(energy);
    std::cout << " " << energy << "/100\n";

    // Display the time bar and its numerical value
    std::cout << " Time     : ";
    printBar(time);
    std::cout << " " << time << "/100\n";

    // Display the players current score
    std::cout << " Score    : " << score << "/520\n";

    // Display the items currently stored in the player's bag
    std::cout << " Bag      : ";

    if (inventoryCount == 0) {
        std::cout << "empty"; // Display empty if there are no items
    }

    // Loop through the inventory and display every collected item
    for (int i = 0; i < inventoryCount; i++) {

        std::cout << getEntityName(inventory[i]);

        // Add a comma between items, but not after the last item
        if (i < inventoryCount - 1) {
            std::cout << ", ";
        }
    }

    std::cout << "\n";

    // Display remaining emergency lift trips if the player has any
    if (emergencyTripsLeft > 0) {
        std::cout << " Emergency Lift 2 trips left: " << emergencyTripsLeft << "\n";
    }

    std::cout << "------------------------------------------\n";
}

// Displays the objects that the player can see in the current room
// floor and room identify the players current location
// const roomEntities means the function cannot modify the room's objects
// entityCount tells how many objects are present in the room
void renderRoomDetails(int floor, int room,
                       const char