#include "database.h"

// Indent by the depth.
static void indent(uint32 depth) {
    for(uint32 i = 0; i < depth; i++) {
        printf("    ");
    }
}

// Print out the identifire scope tree starting at the depth.
static void printIdentTree(xyIdent ident, uint32 depth) {
    indent(depth);
    utSym sym = xyIdentGetSym(ident);
    char *name = "anonymous";
    if(sym != utSymNull) {
        name = utSymGetName(sym);
    }
    printf("%s\n", name);
    depth++;
    xyIdent child;
    xyForeachIdentIdent(ident, child) {
        printIdentTree(child, depth);
    } xyEndIdentIdent;
}

// Print out the identifier scope tree.
void xyPrintIdentTree(xyIdent ident) {
    printIdentTree(ident, 0);
}

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
