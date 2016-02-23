#include "database.h"

// Print a list.
void xyPrintList(xyList list) {
    putchar('(');
    bool firstTime = true;
    xyValue value;
    xyForeachListValue(list, value) {
        if(!firstTime) {
            putchar(' ');
        }
        firstTime = false;
        xyPrintValue(value);
    } xyEndListValue;
    putchar(')');
}

