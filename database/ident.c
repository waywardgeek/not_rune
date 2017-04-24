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
    xyForeachIdentInnerIdent(ident, child) {
        printIdentTree(child, depth);
    } xyEndIdentInnerIdent;
}

// Print out the identifier scope tree.
void xyPrintIdentTree(xyIdent ident) {
    printIdentTree(ident, 0);
}

xyIdent xyIdentCreate(xyIdent outerIdent, xyIdent declIdent, xyToken token) {
    xyIdent ident = xyIdentAlloc();
    xyTokenInsertIdent(token, ident);
    if(outerIdent != xyIdentNull) {
        xyIdentAppendInnerIdent(outerIdent, ident);
    }
    return ident;
}

// Look for the identifier, first in the parent scope, and then it's parents.
xyIdent xyLookupIdent(xyIdent outerIdent, utSym sym) {
    xyIdent ident;
    do {
        ident = xyIdentFindInnerIdent(outerIdent, sym);
        if(ident != xyIdentNull) {
            return ident;
        }
        outerIdent = xyIdentGetOuterIdent(outerIdent);
    } while(outerIdent != xyIdentNull);
    return xyIdentNull;
}
