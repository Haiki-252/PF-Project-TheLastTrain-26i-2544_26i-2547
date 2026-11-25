#include "physics.h"
#include "util.h"

// Movement validation and resource adjustments without structures
using namespace std;

void applyResourceDelta(int& energy, int& time, int deltaEnergy, int deltaTime)
{
    // Adjust energy and time, ensuring they stay within the range of 0 to 100
    // clampInt is made in util.h and used here to make sure energy and time do not exceed the bounds of 0 and 100
    energy = clampInt(energy + deltaEnergy, 0, 100);
    time = clampInt(time + deltaTime, 0, 100);
}
void applyScoreDelta(int& score, int deltaScore)
{
    // Adjust score, ensuring it stays within the range of 0 to 520
    score = clampInt(score + deltaScore, 0, 520);
}
bool hasItem(const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount, char itemSymbol)
{
    // Check if the item is present in the inventory
    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == itemSymbol)
        {
            return true;
        }
    }
    return false;
}
bool addItem(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, char itemSymbol)
{
    // Check if the inventory is full
    if (inventoryCount >= MAX_INVENTORY_CAPACITY)
    {
        return false; // Cannot add item, inventory is full
    }

    // Add the item to the inventory
    inventory[inventoryCount] = itemSymbol;
    inventoryCount++;
    return true; // Item added successfully
}
bool removeItem(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, char itemSymbol)
{
    // Find the item in the inventory
    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == itemSymbol)
        {
            // Shift items to fill the gap left by the removed item
            for (int j = i; j < inventoryCount - 1; j++)
            {
                inventory[j] = inventory[j + 1];
            }
            inventoryCount--;
            return true; // Item removed successfully
        }
    }
    return false; // Item not found in the inventory
}