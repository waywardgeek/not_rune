#include "core_int.h"

// Build the keyword table.
static void buildKeywordTable(void) {
}

// Bind identifiers.
static void bindIdentifiers(xyList prog) {
    xyValue value;
    xyForeachListValue(prog, value) {
        
    } xyEndListValue;
}

// Build an internal representation of the package.  Do symantic checking,
// identifier binding, and type checking.
bool coAnalyze(xyList prog) {
    buildKeywordTable();
    bindIdentifiers(prog);
    return true;
}
