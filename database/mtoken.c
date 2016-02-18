#include "database.h"

// Create a new master token object on the parser.
xyMtoken xyMtokenCreate(xyParser parser, xyMtokenType type, utSym sym) {
    xyMtoken mtoken = xyMtokenAlloc();
    xyMtokenSetType(mtoken, type);
    xyMtokenSetSym(mtoken, sym);
    xyParserAppendMtoken(parser, mtoken);
    return mtoken;
}

