#include "database.h"

xyIdent xyIdentCreate(xyIdent parentScope, utSym sym) {
    xyIdent ident = xyIdentAlloc();
    xyIdentSetSym(ident, sym);
    if(parentScope != xyIdentNull) {
        xyIdentAppendIdent(parentScope, ident);
    }
    return ident;
}

// Look for the identifier, first in the parent scope, and then it's parents.
xyIdent xyLookupIdent(xyIdent parentScope, utSym sym) {
    xyIdent ident;
    do {
        ident = xyIdentFindIdent(parentScope, sym);
        if(ident != xyIdentNull) {
            return ident;
        }
        ident = xyIdentGetIdent(parentScope);
    } while(ident != xyIdentNull);
    return xyIdentNull;
}

// Create a new identifier reference object.
xyIdref xyIdrefCreate(xyIdent ident) {
    xyIdref idref = xyIdrefAlloc();
    xyIdentAppendIdref(ident, idref);
    return idref;
}
