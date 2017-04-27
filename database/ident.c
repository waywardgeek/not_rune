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

// Create an identifier from a token that declares it.
xyIdent xyIdentCreate(xyIdent outerIdent, xyToken token) {
    xyIdent ident = xyIdentAlloc();
    if (token != xyTokenNull) {
        xyIdentInsertToken(ident, token);
        xyIdentSetSym(ident, xyTokenGetSymVal(token));
        xyIdentSetType(ident, xyTokenGetType(token));
    } else {
        xyIdentSetType(ident, XY_IDSCOPE);
    }
    if(outerIdent != xyIdentNull) {
        xyIdentAppendIdent(outerIdent, ident);
    }
    return ident;
}

// Create a symbol not declared by a token.
xyIdent xySymIdentCreate(xyTokenType type, xyIdent outerIdent, utSym sym) {
    xyIdent ident = xyIdentAlloc();
    xyIdentSetSym(ident, sym);
    xyIdentSetType(ident, type);
    if(outerIdent != xyIdentNull) {
        xyIdentAppendIdent(outerIdent, ident);
    }
    return ident;
}

// Look for the identifier, first in the parent scope, and then it's parents.
xyIdent xyLookupIdent(xyIdent outerIdent, utSym sym) {
    xyIdent ident;
    do {
        ident = xyIdentFindIdent(outerIdent, sym);
        if(ident != xyIdentNull) {
            return ident;
        }
        outerIdent = xyIdentGetIdent(outerIdent);
    } while(outerIdent != xyIdentNull);
    return xyIdentNull;
}
