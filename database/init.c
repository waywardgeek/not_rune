#include "database.h"

xyRoot xyTheRoot;
void xyStart(void) {
    xyDatabaseStart();
    xyTheRoot = xyRootAlloc();
}

void xyStop(void) {
    xyDatabaseStop();
}

