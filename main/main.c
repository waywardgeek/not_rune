#include "database.h"
#include "parse.h"

xyRoot xyTheRoot;

int main(int argc, char **argv) {
    utStart();
    xyDatabaseStart();
    xyTheRoot = xyRootAlloc();
    for(int i = 1; i < argc; i++) {
        xpParseGrammar(argv[i]);
    }
    xyDatabaseStop();
    utStop(false);
    return 0;
}
