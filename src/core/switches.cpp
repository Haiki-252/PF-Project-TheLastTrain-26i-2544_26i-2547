#include "switches.h"
#include "physics.h"
#include "util.h"

bool canUseLift1(int fromFloor, int toFloor)
{
    return ((fromFloor == 0 && toFloor == 1) || (fromFloor == 1 && toFloor == 0));
}
// Lift 1 connects only F1 and F2 so (0 to 1 or 1 to 0)
bool canUseLift2(int fromFloor, int toFloor, bool lift2Operational, int emergencyTripsLeft)
{
    if (fromFloor == toFloor)
    {
        return false;
    }

    bool fromValid = (fromFloor == 1 || fromFloor == 3 || fromFloor == 4);
    bool toValid   = (toFloor == 1 || toFloor == 3 || toFloor == 4);

    if (!fromValid || !toValid)
    {
        return false;
    }

    return (lift2Operational || emergencyTripsLeft > 0);
    // lift2Operational is true if the fuse is installed and the player has the lift pass.
    // emergencyTripsLeft > 0 means the player has the emergency card.
}

bool unlockFloor3Door(const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount)
//const char inventory: Passes the player's inventory array in read-only mode (const), so items are neither added nor removed.
//inventoryCount tells the numebr of items present in inventory.
{
    return hasItem(inventory, inventoryCount, ENT_MASTER_KEY);
    //calls function from physics.cpp and loops from 0 to inventoryCount - 1 looking for ENT_MASTER_KEY ('<').
}
// Puts the Fuse into the Control Panel.
// Removes the Fuse from the bag, marks it installed and gives +20 points.
// The player must already be in F3-R4 (handleUseCommand checks that).
bool installFuseControlPanel(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                             bool objectives[NUM_OBJECTIVES], int& score) {
    if (objectives[OBJ_FUSE_INSTALLED]) {
        return false;   // already done
    }
    if (!hasItem(inventory, inventoryCount, ENT_FUSE)) {
        return false;   // no fuse in the bag
    }

    removeItem(inventory, inventoryCount, ENT_FUSE);
    objectives[OBJ_FUSE_INSTALLED] = true;
    applyScoreDelta(score, SCORE_INSTALL_FUSE);

    // Lift 2 works only when the fuse is in AND the player has the Lift Pass
    if (objectives[OBJ_LIFT_PASS]) {
        objectives[OBJ_LIFT2_ACTIVE] = true;
    }
    return true;
}

// The code is right only if all three clues were found AND the code is "427".
bool verifyCabinetCode(const char* enteredCode, const bool objectives[NUM_OBJECTIVES]) {
    if (!objectives[OBJ_CLUE_4] || !objectives[OBJ_CLUE_2] || !objectives[OBJ_CLUE_7]) {
        return false;
    }
    return stringEquals(enteredCode, "427");
}
