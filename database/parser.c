#include "database.h"

// Print out the xyParser.
void xyPrintParser(xyParser parser) {
    printf("Parser Action/GOTO Table\n");
    xyState state;
    xyForeachParserState(parser, state) {
        xyPrintState(state);
    } xyEndParserState;
}

// Create a new parser.
xyParser xyParserCreate(utSym sym) {
    xyParser parser = xyParserAlloc();
    xyParserSetSym(parser, sym);
    xyRootAppendParser(xyTheRoot, parser);
    return parser;
}

