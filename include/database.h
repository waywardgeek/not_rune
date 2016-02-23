#include "xydatabase.h"

// String
xyString xyStringCreate(uint8 *text);

// Parser
xyParser xyParserCreate(utSym sym);
xyAction xyGotoActionCreate(xyState state, xyMtoken mtoken, xyState destState);
xyAction xyShiftActionCreate(xyState state, xyMtoken mtoken, xyState destState);
xyAction xyReduceActionCreate(xyState state, xyMtoken mtoken, xyMtoken reduceMtoken,
    uint32 statesToPop, xyMap map);
xyAction xyAcceptActionCreate(xyState state, xyMtoken mtoken);
void xyPrintParser(xyParser parser);

// Mtoken
xyMtoken xyMtokenCreate(xyParser parser, xyMtokenType type, utSym sym);

// Value
xyValue xySymValueCreate(utSym sym);
xyValue xyIntValueCreate(int64 value);
xyValue xyUintValueCreate(uint64 value);
xyValue xyFloatValueCreate(double value);
xyValue xyStringValueCreate(uint8 *value);
xyValue xyListValueCreate(xyList list);
xyValue xyBoolValueCreate(bool value);
void xyPrintValue(xyValue value);

// List
void xyPrintList(xyList list);

// Map
xyMap xyMapCreate(xyParser parser, xyMapType type);
void xyPrintMap(xyMap map);

// State
void xyPrintState(xyState state);

// Shortcuts
char *xyMtokenGetName(xyMtoken mtoken);
char *xyValueTypeGetName(xyValueType type);

// Globals
extern xyRoot xyTheRoot;
