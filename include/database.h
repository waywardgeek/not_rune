#include "xydatabase.h"

// Initialization.
void xyStart(void);
void xyStop(void);

// Type binding.
void xyRegisterBuiltins(xyParser parser);
xyBindfunc xyRegisterBindfunc(xyParser parser, utSym sym, xyBindfunc bindfunc);
void xyUnregisterBindfunc(xyParser parser, utSym sym, xyBindfunc oldBindfunc);
void xyBuildIdentTree(xyIdent currentScope, xyToken token);
xyToken xyBindToken(xyIdent currentScope, xyToken token);
void xyBindList(xyIdent currentScope, xyList list);
void xyBindType(xyIdent currentScope, xyToken typeToken);

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
xyMtoken xyMtokenCreate(xyParser parser, xyTokenType type, utSym sym);

// Token
xyToken xyIntTokenCreate(xyParser parser, int64 token, uint32 linenum);
xyToken xyFloatTokenCreate(xyParser parser, double token, uint32 linenum);
xyToken xyStringTokenCreate(xyParser parser, uint8 *token, uint32 linenum);
xyToken xyListTokenCreate(xyParser parser, xyList list, uint32 linenum);
xyToken xyBoolTokenCreate(xyParser parser, bool token, uint32 linenum);
xyToken xyKeywordTokenCreate(xyParser parser, utSym sym, uint32 linenum);
xyToken xyIdentTokenCreate(xyParser parser, xyTokenType type, utSym sym, uint32 linenum);
xyToken xyCharTokenCreate(xyParser parser, char *text, uint32 linenum);
xyToken xyEOFTokenCreate(xyParser parser, uint32 linenum);
xyToken xyNewlineTokenCreate(xyParser parser, uint32 linenum);
xyToken xyNullTokenCreate(xyParser parser, uint32 linenum);
char *xyTokenGetText(xyToken token);
void xyPrintToken(xyToken token);
void xyError(xyToken token, char *message, ...);

// List
char *xyListGetText(xyList list);
void xyPrintList(xyList list);

// Map
xyMap xyMapCreate(xyParser parser, xyMapType type);
void xyPrintMap(xyMap map);

// State
void xyPrintState(xyState state);

// Ident
xyIdent xyLookupIdent(xyIdent parentScope, utSym sym);
xyIdent xyIdentCreate(xyIdent outerIdent, xyToken token);
xyIdent xySymIdentCreate(xyTokenType type, xyIdent outerIdent, utSym sym);
void xyPrintIdentTree(xyIdent ident);

// Shortcuts
char *xyMtokenGetName(xyMtoken mtoken);
char *xyTokenTypeGetName(xyTokenType type);

// Globals
extern xyRoot xyTheRoot;
extern xyToken xyArrayType, xyBoolType, xyIntType, xyInt8Type, xyInt16Type;
extern xyToken xyInt32Type, xyInt64Type, xyUintType, xyUint8Type, xyUint16Type;
extern xyToken xyUint32Type, xyUint64Type, xyFloatType, xyStringType, xyCharType;
extern xyToken xyTrueToken, xyFalseToken, xyNullToken;
