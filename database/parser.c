#include "database.h"

// Make a map object.
xyMap xyMapCreate(xyMapType type) {
    xyMap map = xyMapAlloc();
    xyMapSetType(map, type);
    return map;
}

// Create a new parser.
xyParser xyParserCreate(utSym sym) {
    xyParser parser = xyParserAlloc();
    xyParserSetSym(parser, sym);
    xyRootAppendParser(xyTheRoot, parser);
    return parser;
}

