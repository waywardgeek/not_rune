#include "rune.h"

xyRoot xyTheRoot;
utSym xyINTSym, xyUINTSym, xyFLOATSym, xyDOUBLESym, xyBOOLSym, xySTRINGSym, xyLISTSym, xyIDENTSym;

static void initSyms(void) {
    xyINTSym = utSymCreate("INT");
    xyUINTSym = utSymCreate("UINT");
    xyFLOATSym = utSymCreate("FLOAT");
    xyDOUBLESym = utSymCreate("DOUBLE");
    xyBOOLSym = utSymCreate("BOOL");
    xySTRINGSym = utSymCreate("STRING");
    xyLISTSym = utSymCreate("LIST");
    xyIDENTSym = utSymCreate("IDENT");
}

int main(int argc, char **argv) {
    utStart();
    xyDatabaseStart();
    xyTheRoot = xyRootAlloc();
    initSyms();
    xyParseGrammar("rules");
    for(int i = 1; i < argc; i++) {
        xyParse(argv[i]);
    }
    xyDatabaseStop();
    utStop(false);
    return 0;
}
