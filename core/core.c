#include "core_int.h"

FILE *coFile;

// Create a new builtin.
coBuiltin coBuiltinCreate(coBuiltinType type, coBuiltinCategory category, char *name) {
    coBuiltin builtin = coBuiltinAlloc();
    coBuiltinSetSym(builtin, utSymCreate(name));
    coRootInsertBuiltin(xyTheRoot, builtin);
    return builtin;
}

// Create a new builtin type.
coBuiltin coBuiltinCreateType(xyIdent globalScope, coBuiltinType type,
        coBuiltinCategory category, char *name) {
    coBuiltin builtin = coBuiltinCreate(type, category, name);
    xySymIdentCreate(XY_IDTYPE, globalScope, coBuiltinGetSym(builtin));
    return builtin;
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
bool coCompileList(xyToken prog, char *outHFileName, char *outCFileName,
        bool usePointerReferences) {
    coDatabaseStart();
    xyIdent globalScope = xyRootGetGlobalIdent(xyTheRoot);
    utAssert(xyTokenGetType(prog) == XY_LIST);
    xyBindToken(globalScope, prog);
    coWriteHeaderFile(prog, outHFileName);
    coWriteSourceFile(prog, outCFileName);
    coDatabaseStop();
    xyPrintIdentTree(globalScope);
    return true;
}
