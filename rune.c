#include "rune.h"

xyRoot xyTheRoot;
utSym xyINTSym, xyUINTSym, xyFLOATSym, xyDOUBLESym, xyBOOLSym, xySTRINGSym, xyLISTSym, xyIDENTSym;
utSym xyEOFSym, xyEMPTYSym;

static void initSyms(void) {
    xyINTSym = utSymCreate("INT");
    xyUINTSym = utSymCreate("UINT");
    xyFLOATSym = utSymCreate("FLOAT");
    xyDOUBLESym = utSymCreate("DOUBLE");
    xyBOOLSym = utSymCreate("BOOL");
    xySTRINGSym = utSymCreate("STRING");
    xyLISTSym = utSymCreate("LIST");
    xyIDENTSym = utSymCreate("IDENT");
    xyEOFSym = utSymCreate("EOF");
    xyEMPTYSym = utSymCreate("EMPTY");
}

int main(int argc, char **argv) {
    utStart();
    xyDatabaseStart();
    xyTheRoot = xyRootAlloc();
    initSyms();
    for(int i = 1; i < argc; i++) {
        xyParseGrammar(argv[i]);
    }
    xyDatabaseStop();
    utStop(false);
    return 0;
}
