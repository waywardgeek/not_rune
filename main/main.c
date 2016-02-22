#include "database.h"
#include "parsegen.h"
#include "parse.h"

xyRoot xyTheRoot;

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("Usage: rune ruleFile fileToParse\n");
        return 1;
    }
    utStart();
    xyDatabaseStart();
    xyTheRoot = xyRootAlloc();
    xyParser parser = xpParseGrammar(argv[1]);
    FILE *file = fopen(argv[2], "r");
    if(file == NULL) {
        printf("Unable to read file %s\n", argv[2]);
    }
    paParse(file, parser);
    xyDatabaseStop();
    utStop(false);
    return 0;
}
