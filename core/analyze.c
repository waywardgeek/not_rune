#include "core_int.h"

// Bind identifiers.
static void bindIdentifiers(xyList progList) {
    xyToken token;
    xyForeachListToken(progList, token) {
        xyList statementList = xyTokenGetListVal(token);
        xyToken token = xyListGetiToken(statementList, 0);
        utAssert(xyTokenGetType(token) == XY_IDENT);
        utSym sym = xyTokenGetSymVal(token);
        coKeyword keyword = coRootFindKeyword(xyTheRoot, sym);
        if(keyword == coKeywordNull) {
            // Only function calls start without a keyword
            coError(token, "Executable statements are not allowed in the global scope");
        }
        //switch(coKeywordGetType(keyword)) {
        //}
    } xyEndListToken;
}

// Build an internal representation of the package.  Do symantic checking,
// identifier binding, and type checking.
bool coAnalyze(xyList prog) {
    bindIdentifiers(prog);
    return true;
}
