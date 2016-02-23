#include "database.h"

// Create a string.
xyString xyStringCreate(uint8 *text) {
    xyString string = xyStringAlloc();
    xyStringSetText(string, text, strlen((char *)text));
    return string;
}
