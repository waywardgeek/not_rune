#include "database.h"

// Print a concat map.
static void printConcatMap(xyMap map) {
    xyMap left = xyMapGetFirstMap(map);
    xyMap right = xyMapGetNextMapMap(left);
    xyPrintMap(left);
    putchar('.');
    xyPrintMap(right);
}

// Print a list map.
static void printListMap(xyMap map) {
    putchar('(');
    bool firstTime = true;
    xyMap child;
    xyForeachMapMap(map, child) {
        if(!firstTime) {
            putchar(' ');
        }
        firstTime = false;
        xyPrintMap(child);
    } xyEndMapMap;
}

// Print a value map.
static void printValueMap(xyMap map) {
    printf("$%u", xyMapGetPosition(map));
}

// Print a map object.
void xyPrintMap(xyMap map) {
    switch(xyMapGetType(map)) {
    case XY_MAP_CONCAT:
        printConcatMap(map);
        break;
    case XY_MAP_LIST:
        printListMap(map);
        break;
    case XY_MAP_VALUE:
        printValueMap(map);
        break;
    default:
        utExit("Unknown map type");
    }
}

// Make a map object.
xyMap xyMapCreate(xyMapType type) {
    xyMap map = xyMapAlloc();
    xyMapSetType(map, type);
    return map;
}

