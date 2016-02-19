#include "xydatabase.h"

// String
xyString xyStringCreate(uint8 *text, uint32 len);

// Parser
xyParser xyParserCreate(utSym sym);
xyAction xyGotoActionCreate(xyState state, xyMtoken mtoken, xyState destState);
xyAction xyShiftActionCreate(xyState state, xyMtoken mtoken, xyState destState);
xyAction xyReduceActionCreate(xyState state, xyMtoken mtoken, xyMtoken reduceMtoken, uint32 statesToPop);
xyAction xyAcceptActionCreate(xyState state, xyMtoken mtoken);

// Mtoken
xyMtoken xyMtokenCreate(xyParser parser, xyMtokenType type, utSym sym);

// Shortcuts
static inline char *xyMtokenGetName(xyMtoken mtoken) {return utSymGetName(xyMtokenGetSym(mtoken));}
char *xyValueTypeGetName(xyValueType type);

// Debugging
void xyPrintAGTable(xyAGTable agtable);

// Globals
extern xyRoot xyTheRoot;
