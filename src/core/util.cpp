#include "util.h"
#include <iostream>

// Primitive helper utilities, clamping, and screen control

// Small helper: turns a capital letter into a small letter ('M' -> 'm').
// Other characters are returned unchanged
// Converts uppercase letters to lowercase using ASCII values.
static char toSmallLetter(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    return c;
}

// Small helper: true for space, tab and new-line characters
static bool isSpaceChar(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// Keeps value between minVal and maxVal
// like in clampInt(120, 0, 100) gives 100
int clampInt(int val, int minVal, int maxVal) {
    if (val < minVal) {
        return minVal;
    }
    if (val > maxVal) {
        return maxVal;
    }
    return val;
}

// Removes spaces from the start and the end of a word
// like "  move F1-R2  " becomes "move F1-R2"
void trimWhitespace(char* str) {
    // Checks whether the pointer is empty before using it
    if (str == nullptr) {
        return;
    }

    // Find the length
    int length = 0;
    while (str[length] != '\0') {
        length++;
    }

    // Find the first real character
    int start = 0;
    while (start < length && isSpaceChar(str[start])) {
        start++;
    }

    // Find the last real character
    int end = length - 1;
    while (end >= start && isSpaceChar(str[end])) {
        end--;
    }

    // move the middle part to the front and end it with '\0'
    // copies the remaining characters to the beginning of the array
    int j = 0;
    for (int i = start; i <= end; i++) {
        str[j] = str[i];
        j++;
    }
    str[j] = '\0';
}

// checks if two words are the same
// small and capital letters count as equal, so "MOVE" and "move" are the same.
// compares characters one by one after converting them to lowercase
bool stringEquals(const char* a, const char* b) {
    if (a == nullptr || b == nullptr) {
        return false;
    }

    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (toSmallLetter(a[i]) != toSmallLetter(b[i])) {
            return false;
        }
        i++;
    }

    // Both words must end at the same place
    return a[i] == '\0' && b[i] == '\0';
}

// Prints a title between two lines of '='
void printHeader(const char* title) {
    std::cout << "==========================================\n";
    std::cout << " " << title << "\n";
    std::cout << "==========================================\n";
}

// Uses terminal escape codes to clear the screen and move the cursor to the top
void clearScreen() {
    std::cout << "\033[2J\033[H";
}

// Turns room text like "F3-R2" into 0-based numbers: floor = 2, room = 1
// Returns false if the text is not a valid room
bool parseRoomId(const char* text, int& floor, int& room) {
    if (text == nullptr) {
        return false;
    }

    // The text must be exactly 5 characters long, like F3-R2
    int length = 0;
    while (text[length] != '\0') {
        length++;
    }
    if (length != 5) {
        return false;
    }

    // Checking the shape: F, digit, -, R, digit
    if (toSmallLetter(text[0]) != 'f' || text[2] != '-' || toSmallLetter(text[3]) != 'r') {
        return false;
    }
    if (text[1] < '1' || text[1] > '5') {      // Floors 1 to 5
        return false;
    }
    if (text[4] < '1' || text[4] > '4') {      // Rooms 1 to 4
        return false;
    }

    // Converts the floor and room numbers into array indexes starting from 0
    // '3' - '1' = 2, so F3 becomes floor index 2
    floor = text[1] - '1';
    room = text[4] - '1';
    return true;
}