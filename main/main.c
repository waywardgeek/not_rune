#include "database.h"
#include "parsegen.h"
#include "parse.h"
#include "core.h"

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("Usage: rune ruleFile fileToParse\n");
        return 1;
    }
    utStart();
    xyStart();
    xyParser parser = xpParseGrammar(argv[1]);
    xyRegisterBuiltins(parser);
    FILE *file = fopen(argv[2], "r");
    if(file == NULL) {
        printf("Unable to read file %s\n", argv[2]);
        return 1;
    }
    xyToken token = paParse(file, parser);
    printf("Result:\n");
    xyPrintToken(token);
    putchar('\n');
    utAssert(xyTokenGetType(token) == XY_LIST);
    coCompileList(token, "output.h", "output.c", true);
    xyStop();
    utStop(false);
    return 0;
}
