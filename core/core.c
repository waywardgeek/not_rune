#include "core_int.h"

FILE *coFile;

// Create a new keyword.
coKeyword coKeywordCreate(coKeywordType type, char *name) {
    coKeyword keyword = coKeywordAlloc();
    coKeywordSetSym(keyword, utSymCreate(name));
    coRootInsertKeyword(xyTheRoot, keyword);
    return keyword;
}

// Build the keyword table.
static void buildKeywords(void) {
    coKeywordCreate(CO_KWIMPORT, "import");
    coKeywordCreate(CO_KWASSIGN, "=");
    coKeywordCreate(CO_KWIF, "if");
    coKeywordCreate(CO_KWWHILE, "while");
    coKeywordCreate(CO_KWDO, "do");
    coKeywordCreate(CO_KWRETURN, "return");
    coKeywordCreate(CO_KWDELETE, "delete");
    coKeywordCreate(CO_KWSWITCH, "switch");
    coKeywordCreate(CO_KWDEFAULT, "default");
    coKeywordCreate(CO_KWVAR, "var");
    coKeywordCreate(CO_KWFUNC, "func");
    coKeywordCreate(CO_KWCLASS, "class");
    coKeywordCreate(CO_KWARRAY, "array");
    coKeywordCreate(CO_KWINDEX, "index");
    coKeywordCreate(CO_KWOR, "||");
    coKeywordCreate(CO_KWAND, "&&");
    coKeywordCreate(CO_KWNOT, "!");
    coKeywordCreate(CO_KWEQ, "==");
    coKeywordCreate(CO_KWNE, "!=");
    coKeywordCreate(CO_KWGT, ">");
    coKeywordCreate(CO_KWLT, "<");
    coKeywordCreate(CO_KWGE, ">=");
    coKeywordCreate(CO_KWLE, "<=");
    coKeywordCreate(CO_KWMINUS, "-");
    coKeywordCreate(CO_KWPLUS, "+");
    coKeywordCreate(CO_KWMUL, "*");
    coKeywordCreate(CO_KWBITAND, "&");
    coKeywordCreate(CO_KWBITOR, "|");
    coKeywordCreate(CO_KWBITXOR, "^");
    coKeywordCreate(CO_KWDIV, "/");
    coKeywordCreate(CO_KWMOD, "%");
    coKeywordCreate(CO_KWLSHIFT, "<<");
    coKeywordCreate(CO_KWRSHIFT, ">>");
    coKeywordCreate(CO_KWCOMP, "~");
    coKeywordCreate(CO_KWNEW, "new");
    coKeywordCreate(CO_KWDOT, "dot");
}

// Just create a global identifier for the string.
static inline void addGlobal(xyIdent globalScope, char *text) {
    xyIdentCreate(globalScope, utSymCreate(text));
}

// Build global identifiers.
static void buildGlobalIdents(xyIdent globalScope) {
    addGlobal(globalScope, "bool");
    addGlobal(globalScope, "true");
    addGlobal(globalScope, "false");
    // TODO: add at least uint128, or preferably arbitrary width
    addGlobal(globalScope, "int");
    addGlobal(globalScope, "int8");
    addGlobal(globalScope, "int16");
    addGlobal(globalScope, "int32");
    addGlobal(globalScope, "int64");
    addGlobal(globalScope, "uint");
    addGlobal(globalScope, "uint8");
    addGlobal(globalScope, "uint16");
    addGlobal(globalScope, "uint32");
    addGlobal(globalScope, "uint64");
    addGlobal(globalScope, "float");
    addGlobal(globalScope, "string");
    addGlobal(globalScope, "char");
    addGlobal(globalScope, "null");
}

// Write the header file.
bool coWriteHeaderFile(xyToken prog, char *outHFileName) {
    coFile = fopen(outHFileName, "w");
    if(coFile == NULL) {
        utExit("Unable to open C header file %s for writing", outHFileName);
    }
    //writeHeaderFile(prog);
    fclose(coFile);
    return true;
}

// Write the source file.
bool coWriteSourceFile(xyToken prog, char *outCFileName) {
    coFile = fopen(outCFileName, "w");
    if(coFile == NULL) {
        utExit("Unable to open C source file %s for writing", outCFileName);
    }
    //writeSourceFile(prog);
    fclose(coFile);
    return true;
}

// Compile the list to C.  Object references can be either pointers or
// integers, depending on the usePointerReferenes parameter.  When true, all
// class properties are stored in arrays, indexed by the reference.  When
// false, class properties are stored in contiguous structures, and references
// are pointers.
bool coCompileList(xyToken prog, char *outHFileName, char *outCFileName, bool usePointerReferences) {
    coDatabaseStart();
    buildKeywords();
    xyIdent globalScope = xyIdentCreate(xyIdentNull, utSymNull);
    xyRootSetGlobalIdent(xyTheRoot, globalScope);
    buildGlobalIdents(globalScope);
    coBindIdentifiers(prog, globalScope);
    coWriteHeaderFile(prog, outHFileName);
    coWriteSourceFile(prog, outCFileName);
    coDatabaseStop();
    xyPrintIdentTree(globalScope);
    return true;
}
