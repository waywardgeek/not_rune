#include "database.h"

// Create a new parser.
xyParser xyParserCreate(utSym sym) {
    xyParser parser = xyParserAlloc();
    xyParserSetSym(parser, sym);
    xyRootAppendParser(xyTheRoot, parser);
    return parser;
}

