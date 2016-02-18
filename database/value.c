#include "database.h"

// Get the name of an xyValueType.
char *xyValueTypeGetName(xyValueType type) {
    switch(type) {
    case XY_INT: return "INT";
    case XY_UINT: return "UINT";
    case XY_FLOAT: return "FLOAT";
    case XY_DOUBLE: return "DOUBLE";
    case XY_BOOL: return "BOOL";
    case XY_STRING: return "STRING";
    case XY_LIST: return "LIST";
    case XY_SYM: return "SYM";
    default:
        utExit("Unknown value type");
    }
    return NULL; // Dummy return
}
