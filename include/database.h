#include "xydatabase.h"

// String
xyString xyStringCreate(uint8 *text, uint32 len);

// Parser
xyParser xyParserCreate(utSym sym);

// Mtoken
xyMtoken xyMtokenCreate(xyParser parser, xyMtokenType type, utSym sym);

// Shortcuts
static inline char *xyMtokenGetName(xyMtoken mtoken) {return utSymGetName(xyMtokenGetSym(mtoken));}
char *xyValueTypeGetName(xyValueType type);

// Globals
extern xyRoot xyTheRoot;
