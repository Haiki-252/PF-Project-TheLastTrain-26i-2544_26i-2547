#include "switches.h"
#include "physics.h"
#include "util.h"
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
