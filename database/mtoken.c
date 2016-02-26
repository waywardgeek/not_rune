#include "database.h"

// Return a text name for this mtoken.
char *xyMtokenGetName(xyMtoken mtoken) {
    switch(xyMtokenGetType(mtoken)) {
    case XY_NONTERM:
    case XY_KEYWORD:
        return utSymGetName(xyMtokenGetSym(mtoken));
    case XY_INT: return "INTEGER";
    case XY_BOOL: return "BOOL";
    case XY_FLOAT: return "FLOAT";
    case XY_STRING: return "STRING";
    case XY_CHAR: return "CHAR";
    case XY_IDENT: return "IDENT";
    case XY_NEWLINE: return "NEWLINE";
    case XY_EOF: return "EOF";
    default:
        utExit("Unknown mtoken type");
    }
    return NULL; // Dummy return
}

// Create a new master token object on the parser.
xyMtoken xyMtokenCreate(xyParser parser, xyTokenType type, utSym sym) {
    xyMtoken mtoken = xyParserFindMtoken(parser, type, sym);
    if(mtoken != xyMtokenNull) {
        return mtoken;
    }
    mtoken = xyMtokenAlloc();
    xyMtokenSetType(mtoken, type);
    xyMtokenSetSym(mtoken, sym);
    xyParserAppendMtoken(parser, mtoken);
    return mtoken;
}

