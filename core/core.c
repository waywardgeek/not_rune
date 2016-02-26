#include "core_int.h"

static FILE *coFile;

// Write the header file.
bool coWriteHeaderFile(xyList prog, char *outHFileName) {
    coFile = fopen(outHFileName, "r");
    if(coFile == NULL) {
        utExit("Unable to open C header file %s for writing", outHFileName);
        return false;
    }
    //writeHeaderFile(prog);
    fclose(coFile);
    return true;
}

// Write the source file.
bool coWriteSourceFile(xyList prog, char *outCFileName) {
    coFile = fopen(outCFileName, "r");
    if(coFile == NULL) {
        utExit("Unable to open C source file %s for writing", outCFileName);
        return false;
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
bool coCompileList(xyList prog, char *outHFileName, char *outCFileName, bool usePointerReferences) {
    if(!coAnalyze(prog) ||
            !coWriteHeaderFile(prog, outHFileName) ||
            !coWriteSourceFile(prog, outCFileName)) {
        return false;
    }
    return true;
}
