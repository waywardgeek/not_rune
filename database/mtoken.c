#include "database.h"

// Return a text name for this mtoken.
char *xyMtokenGetName(xyMtoken mtoken) {
    switch(xyMtokenGetType(mtoken)) {
    case XY_TOK_NONTERM:
    case XY_TOK_KEYWORD:
        return utSymGetName(xyMtokenGetSym(mtoken));
    case XY_TOK_INTEGER: return "INTEGER";
    case XY_TOK_FLOAT: return "FLOAT";
    case XY_TOK_STRING: return "STRING";
    case XY_TOK_CHAR: return "CHAR";
    case XY_TOK_IDENT: return "IDENT";
    case XY_TOK_OPERATOR: return "OPERATOR";
    case XY_TOK_COMMENT: return "COMMENT";
    case XY_TOK_NEWLINE: return "NEWLINE";
    case XY_TOK_BEGIN: return "BEGIN";
    case XY_TOK_END: return "END";
    case XY_TOK_EOF: return "EOF";
    default:
        utExit("Unknown mtoken type");
    }
}

// Create a new master token object on the parser.
xyMtoken xyMtokenCreate(xyParser parser, xyMtokenType type, utSym sym) {
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

