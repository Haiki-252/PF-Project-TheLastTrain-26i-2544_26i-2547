#include "level.h"
#include "util.h"
#include <fstream>
#include <cstring>

// Modular implementation using primitive arrays for map parsing and verification
using namespace std;
bool loadBuildingMap(const char* filename,
                     char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                     char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                     int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    ifstream file(filename);
    if (!file.is_open())
    {
        return false; // Failed to open the file
    }
    for (int f = 0; f < NUM_FLOORS; f++)   // 0,1,2,3,4 all 5 floors
    {
        for (int c = 0; c < ROOMS_PER_FLOOR - 1; c++) // 0,1,2 all 3 horizontal connections
            {
                horizontalConn[f][c] = CONN_NONE; // Initialize to no horizontal connection or space
            }
        for (int r =0; r < ROOMS_PER_FLOOR; r++)  // 0,1,2,3 all 4 rooms
        {
            entityCounts[f][r]=0;
            for (int e = 0; e < MAX_ENTITIES_PER_ROOM; e++) // 0,1,2 all 3 horizontal connections
            {
                roomEntities[f][r][e] = ENT_NONE; // // all entity slots in the room
            }
        }
    }
    int roomsFound = 0;
    char line[256];
    while (file.getline(line,256))
    {
        
        // So that when the building.txt file is getting read it only reads till the building map.
        // And if it comes to line  19 (D) or line 29 (Objects) it would break and stop so it doesnt count that as the extra rooms or floors.
        if (strncmp(line, "D = ", 4) == 0 || strncmp(line, "OBJECTS:", 8) == 0)
        {
            break;
        }
        //strncmp : string n-compare from <cstring> It compares the 1st n characters of 2 c-strings(char arrays).
        // we used 8 size in strncmp because the word "OBJECTS:" has 8 characters in it.
        //strncmp(const char* str1, const char* str2, size_t n)
        // str1: 1st string to check, str2: 2nd string to compare against, n: how many number of characters from the start to compare.
        // it gives us 0 when str1 an str2 are identical and non-zero when they are different.
        // 2. Scan the line for room labels (pattern: F1-R1 ... F5-R4)
        int len = static_cast<int>(strlen(line)); 
        //strnlen tells us how many characters are in line before '\0'. If line holds "HELLO\0", strlen(line) returns 5.
        // static_cast converts a value from one data type to another.
        // as strlen() return unsigned int type (called size_t) so we use static cast to changeits data type to int to avoid any warnings when we r gonna compare it later on with other integers.
        int labelCol[ROOMS_PER_FLOOR];
        // 1D int array of size 4
        int labelsOnLine = 0;
        //per-line counter that tracks how many room labels (F?-R?) we have spotted so far on the single line currently stored in line.
        // It is reset to 0 at the start of each new line, and incremented each time we find a valid room label. When it reaches 4, we know we have found all 4 room labels on this line.
        //Filters out non-floor lines: Right after scanning the line, we check if (labelsOnLine != ROOMS_PER_FLOOR) { continue; }
        int floorIdx = -1;
        for (int i = 0; i <= len - 5; i++)
        // i <= len - 5 stops safely 5 characters before the end of line so line[i + 4] never reads past '\0'.
        {
            if (line[i] == 'F' &&
                line[i + 1] >= '1' && line[i + 1] <= '5' &&
                line[i + 2] == '-' &&
                line[i + 3] == 'R' &&
                line[i + 4] >= '1' && line[i + 4] <= '4')
                // F1-R1, F1-R2, F1-R3, F1-R4, F2-R1, ..., F5-R4
            {
                if (labelsOnLine < ROOMS_PER_FLOOR)
                {
                    labelCol[labelsOnLine] = i;
                    floorIdx = line[i + 1] - '1'; // converts '1'..'5' to 0..4 cuz it starts from -1.
                    labelsOnLine++;
                    roomsFound++;
                }
            }
        }
        // If this line did not contain all 4 room labels (e.g. title or border line), skip it
        if (labelsOnLine != ROOMS_PER_FLOOR)
        {
            continue;
        }
        // Next: read the 3 horizontal connections between labelCol[0..3]
        for (int connIdx = 0; connIdx < ROOMS_PER_FLOOR - 1; connIdx++)
        {
            for (int k = labelCol[connIdx] + 5; k < labelCol[connIdx + 1]; k++)
            {
        // Skip "L1" because that is Lift 1, not a Locked Door 'L'
                if (line[k] == 'L' && line[k + 1] == '1')
                {
                    k++;
                    continue;
                }

                if (line[k] == CONN_DOOR || line[k] == CONN_WINDOW ||
                line[k] == CONN_LOCKED_DOOR || line[k] == CONN_SOLID_WALL)
                {
                    horizontalConn[floorIdx][connIdx] = line[k];
                    break;
                }
            }
        }
        // and then read the very next line for this floor's items!
        char itemLine[256];
        if (!file.getline(itemLine, 256))
        {
            return false; // Unexpected end of file right after a floor header
        }

        int itemLen = static_cast<int>(strlen(itemLine));

        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            int startCol = (r == 0) ? 0 : labelCol[r];
            int endCol = (r < ROOMS_PER_FLOOR - 1) ? labelCol[r + 1] : itemLen;

            for (int col = startCol; col < endCol && col < itemLen; col++)
            {
                char ch = itemLine[col];
                if (ch == ' ' || ch == '|' || ch == '\r' || ch == '\n')
                {
                    continue;
                }

                if (entityCounts[floorIdx][r] < MAX_ENTITIES_PER_ROOM)
                {
                    roomEntities[floorIdx][r][entityCounts[floorIdx][r]] = ch;
                    entityCounts[floorIdx][r]++;
                }
            }
        }
    }
    file.close();
    return (roomsFound == NUM_FLOORS * ROOMS_PER_FLOOR);
}
bool loadInitialState(const char* filename,
                      int& playerFloor, int& playerRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        return false;
    }
    // Initialize defaults safely
    playerFloor = -1; // -1 signals PLAYER_SELECTS
    playerRoom = -1;
    energy = 100;
    time = 50;
    score = 0;
    inventoryCount = 0;
    emergencyTripsLeft = 3; // Default emergency trips unless overridden in file
    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++)
    {
        inventory[i] = ENT_NONE;
    } //this loop clears out every slot in the inventory array by setting it to ENT_NONE
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        objectives[i] = false;
    } // this loop resets every game objective flag (0 through 15, such as OBJ_MASTER_KEY, OBJ_FUSE_IN) to false
    char line[256];
    while (file.getline(line, 256))
    {
        // Strip trailing '\r' in case file has Windows CRLF line endings
        int len = static_cast<int>(strlen(line));
        //static_cast converts data type
        while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n' || line[len - 1] == ' '))
        {
            line[len - 1] = '\0';
            len--;
        }
        //discards invisible whitespace and Windows line-ending char (\r, \n, and ' ') from the end of each line read from initial_state.txt.
        if (len == 0)
        {
            continue;
        }
        //skips empty or whitespace-only lines in initial_state.txt and skips to next iteration
        if (strncmp(line, "START_ROOM:", 11) == 0)
        {
            const char* val = line + 11; //val is a pointer
            //Moves the pointer val 11 characters forward (right past the :)
            while (*val == ' ')
            { val++;}
            //skips any space soo val points to the first non-space character after "START_ROOM:"
            if (strncmp(val, "PLAYER_SELECTS", 14) == 0)
            {
                playerFloor = -1;
                playerRoom = -1;
            }
            //-1 values signal to main_core.cpp that the game should prompt the user to pick a starting floor/room and validate it with validateStartingRoom()
            else if (val[0] == 'F' && val[1] >= '1' && val[1] <= '5' &&
                     val[2] == '-' && val[3] == 'R' && val[4] >= '1' && val[4] <= '4')
            {
                playerFloor = val[1] - '1';
                playerRoom = val[4] - '1';
            }
            //converts the ASCII digits '1'–'5' and '1'–'4' into 0-based array indices (0–4 and 0–3) using - '1'
        }
        else if (strncmp(line, "INITIAL_ENERGY:", 15) == 0)
        {
            const char* p = line + 15;
            while (*p == ' ')
            { p++; }
            int val = 0;
            while (*p >= '0' && *p <= '9')
            {
                val = val * 10 + (*p - '0');
                p++;
            }
            energy = val;
        }
        else if (strncmp(line, "INITIAL_TIME:", 13) == 0)
        {
            const char* p = line + 13;
            while (*p == ' ')
            { p++; }
            int val = 0;
            while (*p >= '0' && *p <= '9')
            {
                val = val * 10 + (*p - '0');
                p++;
            }
            time = val;
        }
        else if (strncmp(line, "INITIAL_SCORE:", 14) == 0)
        {
            const char* p = line + 14;
            while (*p == ' ') p++;
            int sign = 1;
            if (*p == '-')
            {
                sign = -1;
                p++;
            }
            int val = 0;
            while (*p >= '0' && *p <= '9')
            {
                val = val * 10 + (*p - '0');
                p++;
            }
            score = val * sign;
        }
        else if (strncmp(line, "EMERGENCY_TRIPS:", 16) == 0)
        {
            const char* p = line + 16;
            while (*p == ' ') p++;
            int val = 0;
            while (*p >= '0' && *p <= '9')
            {
                val = val * 10 + (*p - '0');
                p++;
            }
            emergencyTripsLeft = val;
        }
        else if (strncmp(line, "INITIAL_INVENTORY:", 18) == 0)
        {
            const char* val = line + 18;
            while (*val == ' ') val++;

            if (strncmp(val, "NONE", 4) != 0)
            {
                for (int i = 0; val[i] != '\0'; i++)
                {
                    if (val[i] != ' ' && val[i] != ',' && inventoryCount < MAX_INVENTORY_CAPACITY)
                    {
                        inventory[inventoryCount++] = val[i];
                    }
                }
            }
        }
        else if (strncmp(line, "PASSENGER_RESCUED:", 18) == 0)
        {
            const char* val = line + 18;
            while (*val == ' ') val++;
            objectives[OBJ_PASSENGER_RESCUED] = (strncmp(val, "YES", 3) == 0);
        }
        else if (strncmp(line, "CAT_RESCUED:", 12) == 0)
        {
            const char* val = line + 12;
            while (*val == ' ') val++;
            objectives[OBJ_CAT_SECURED] = (strncmp(val, "YES", 3) == 0);
        }
        else if (strncmp(line, "CABINET_UNLOCKED:", 17) == 0)
        {
            const char* val = line + 17;
            while (*val == ' ') val++;
            objectives[OBJ_CABINET_UNLOCKED] = (strncmp(val, "YES", 3) == 0);
        }
        else if (strncmp(line, "TRAIN_KEY_COLLECTED:", 20) == 0)
        {
            const char* val = line + 20;
            while (*val == ' ') val++;
            objectives[OBJ_TRAIN_KEY] = (strncmp(val, "YES", 3) == 0);
        }
        else if (strncmp(line, "FUSE_INSTALLED:", 15) == 0)
        {
            const char* val = line + 15;
            while (*val == ' ') val++;
            objectives[OBJ_FUSE_INSTALLED] = (strncmp(val, "YES", 3) == 0);
        }
        else if (strncmp(line, "LIFT_PASS_COLLECTED:", 20) == 0)
        {
            const char* val = line + 20;
            while (*val == ' ') val++;
            objectives[OBJ_LIFT_PASS] = (strncmp(val, "YES", 3) == 0);
        }
        else if (strncmp(line, "TRAIN_PREPARED:", 15) == 0)
        {
            const char* val = line + 15;
            while (*val == ' ') val++;
            objectives[OBJ_TRAIN_PREPARED] = (strncmp(val, "YES", 3) == 0);
        }
        else if (strncmp(line, "CLUES_COLLECTED:", 16) == 0)
        {
            const char* val = line + 16;
            while (*val == ' ') val++;
            if (strncmp(val, "NONE", 4) != 0)
            {
                for (int i = 0; val[i] != '\0'; i++)
                {
                    if (val[i] == ENT_CLUE_4) objectives[OBJ_CLUE_4] = true;
                    if (val[i] == ENT_CLUE_2) objectives[OBJ_CLUE_2] = true;
                    if (val[i] == ENT_CLUE_7) objectives[OBJ_CLUE_7] = true;
                }
            }
        }
    }

    file.close();
    return true;
}
bool validateStartingRoom(int startFloor, int startRoom,
                          const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1])
{
    // Bounds check (0-indexed: floors 0..4, rooms 0..3)
    if (startFloor < 0 || startFloor >= NUM_FLOORS ||
        startRoom < 0 || startRoom >= ROOMS_PER_FLOOR)
    {
        return false;
    }
    //Room 0 (R1) always has staircase/lift access, and R4 on floors 2, 4, 5 (indices 1, 3, 4) has Lift 2
    if (startRoom == 0)
    {
        return true;
    }
    if (startRoom == ROOMS_PER_FLOOR - 1 &&
        (startFloor == 1 || startFloor == 3 || startFloor == 4))
    {
        return true;
    }
    //Check if at least one adjacent horizontal connection (left or right) is passable (not '#' or ' ')
    bool hasLeftExit = (startRoom > 0) &&
                       (horizontalConn[startFloor][startRoom - 1] != CONN_SOLID_WALL) &&
                       (horizontalConn[startFloor][startRoom - 1] != CONN_NONE);

    bool hasRightExit = (startRoom < ROOMS_PER_FLOOR - 1) &&
                        (horizontalConn[startFloor][startRoom] != CONN_SOLID_WALL) &&
                        (horizontalConn[startFloor][startRoom] != CONN_NONE);

    return (hasLeftExit || hasRightExit);
}
const char* getEntityName(char entitySymbol)
{
    // Return the name of the entity based on its symbol
    switch (entitySymbol)
    {
        case ENT_MASTER_KEY:
            return "Master Key";
        case ENT_ENERGY_PACK:
            return "Energy Pack";
        case ENT_EMERGENCY_TOOLKIT:
            return "Emergency Toolkit";
        case ENT_CLUE_4:
            return "Clue Digit 4";
        case ENT_FUSE:
            return "Fuse";
        case ENT_CAT:
            return "Stranded Cat";
        case ENT_CONTROL_PANEL:
            return "Control Panel Terminal";
        case ENT_LIFT_PASS:
            return "Lift Pass";
        case ENT_PASSENGER:
            return "Stranded Passenger";
        case ENT_MEDICAL_KIT:
            return "Medical Kit";
        case ENT_EMERGENCY_CARD:
            return "Emergency Access Card";
        case ENT_CLUE_2 :
            return "Clue Digit 2";
        case ENT_CLUE_7:
            return "Clue Digit 7";
        case ENT_CABINET_TRAIN_KEY:
            return "Cabinet / Train Key";
        case ENT_CAT_CARRIER:
            return "Cat Carrier";
        case ENT_OBSERVATION_TOKEN:
            return "Observation Token";
        default:
            return "Unknown";
    }
}