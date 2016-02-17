#include "rune.h"

// Create a string.
xyString xyStringCreate(uint8 *text, uint32 len) {
    xyString string = xyStringAlloc();
    xyStringSetText(string, text, len);
    return string;
}
