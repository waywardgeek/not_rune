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

// Print a value;
void xyPrintValue(xyValue value) {
    switch(xyValueGetType(value)) {
    case XY_INT:
        printf("%lld", xyValueGetIntVal(value));
        break;
    case XY_UINT:
        printf("%llu", xyValueGetUintVal(value));
        break;
    case XY_FLOAT:
        printf("%f", xyValueGetFloatVal(value));
        break;
    case XY_DOUBLE:
        printf("%f", xyValueGetDoubleVal(value));
        break;
    case XY_BOOL:
        printf("%s", xyValueBoolVal(value)? "true" : "false");
        break;
    case XY_STRING:
        printf("%s", xyStringGetText(xyValueGetStringVal(value)));
        break;
    case XY_LIST:
        xyPrintList(xyValueGetListVal(value));
        break;
    case XY_SYM:
        printf("%s", utSymGetName(xyValueGetSymVal(value)));
        break;
    default:
        utExit("Unknown value type");
    }
}

static inline xyValue valueCreate(xyValueType type, uint32 linenum) {
    xyValue value = xyValueAlloc();
    xyValueSetType(value, type);
    xyValueSetLinenum(value, linenum);
    return value;
}

// Create a new sym value.
xyValue xySymValueCreate(utSym sym, uint32 linenum) {
    xyValue value = valueCreate(XY_SYM, linenum);
    xyValueSetSymVal(value, sym);
    return value;
}

// Create a new int value.
xyValue xyIntValueCreate(int64 intVal, uint32 linenum) {
    xyValue value = valueCreate(XY_INT, linenum);
    xyValueSetIntVal(value, intVal);
    return value;
}

// Create a new uint value.
xyValue xyUintValueCreate(uint64 uintVal, uint32 linenum) {
    xyValue value = valueCreate(XY_UINT, linenum);
    xyValueSetUintVal(value, uintVal);
    return value;
}

// Create a new float value.
xyValue xyFloatValueCreate(double floatVal, uint32 linenum) {
    xyValue value = valueCreate(XY_FLOAT, linenum);
    xyValueSetFloatVal(value, floatVal);
    return value;
}

// Create a new string value.
xyValue xyStringValueCreate(uint8 *stringVal, uint32 linenum) {
    xyValue value = valueCreate(XY_STRING, linenum);
    xyString string = xyStringCreate(stringVal);
    xyValueSetStringVal(value, string);
    return value;
}

// Create a new list value.
xyValue xyListValueCreate(xyList list, uint32 linenum) {
    xyValue value = valueCreate(XY_LIST, linenum);
    xyValueSetListVal(value, list);
    return value;
}

// Create a new bool value.
xyValue xyBoolValueCreate(bool boolVal, uint32 linenum) {
    xyValue value = valueCreate(XY_BOOL, linenum);
    xyValueSetBoolVal(value, boolVal);
    return value;
}
