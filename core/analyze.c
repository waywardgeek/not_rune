#include "core_int.h"

// Bind identifiers.
static void bindIdentifiers(xyList progList) {
    xyValue value;
    xyForeachListValue(progList, value) {
        xyList statementList = xyValueGetListVal(value);
        xyValue value = xyListGetiValue(statementList, 0);
        utAssert(xyValueGetType(value) == XY_SYM);
        utSym sym = xyValueGetSymVal(value);
        coKeyword keyword = coRootFindKeyword(xyTheRoot, sym);
        if(keyword == coKeywordNull) {
            // Only function calls start without a keyword
            coError(value, "Executable statements are not allowed in the global scope");
        }
        //switch(coKeywordGetType(keyword)) {
        //}
    } xyEndListValue;
}

// Build an internal representation of the package.  Do symantic checking,
// identifier binding, and type checking.
bool coAnalyze(xyList prog) {
    bindIdentifiers(prog);
    return true;
}
