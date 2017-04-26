#include "core_int.h"

// Find any scoped identifier in the list.  It is illegal to have more than
// one, or to have one in a scoped list since scoped lists are anonymous
// namespaces.
static xyToken findScopedIdentToken(xyList list) {
    xyToken token;
    xyForeachListToken(list, token) {
        xyTokenType type = xyTokenGetType(token);
        if(type == XY_IDSCOPE || type == XY_IDFUNC || type == XY_IDTYPE) {
            return token;
        }
    } xyEndListToken;
    return xyTokenNull;
}

// Bind identifiers recursively under this ident's scope.
static void bindListIdentifiers(xyList list, xyIdent parentScope) {
    xyToken token = findScopedIdentToken(list);
    if(token != xyTokenNull) {
        utAssert(xyListGetIdent(list) == xyIdentNull);
        utAssert(xyTokenGetIdent(token) == xyIdentNull);
        parentScope = xyIdentCreate(parentScope, token);
        xyListSetIdent(list, parentScope);
    }
    xyForeachListToken(list, token) {
        coBindIdentifiers(token, parentScope);
    } xyEndListToken;
}

// Find the type of an expression.  In the core syntax, all expressions are
// either literals or function calls.  If it is a function call, then the type
// is just the function's type expression, or null if it has no type.  If it is
// a literal, the type is the type of the literal, which is represeted by the
// literal token itself.
static xyIdent findType(xyToken token, xyIdent parentScope) {
    xyList list;
    xyToken refToken;
    switch (xyTokenGetType(token)) {
    case XY_ARRAY: return xyArrayType;
    case XY_BOOL: return xyBoolType;
    case XY_INT: return xyIntType;
    case XY_INT8: return xyInt8Type;
    case XY_INT16: return xyInt16Type;
    case XY_INT32: return xyInt32Type;
    case XY_INT64: return xyInt64Type;
    case XY_UINT: return xyUintType;
    case XY_UINT8: return xyUint8Type;
    case XY_UINT16: return xyUint16Type;
    case XY_UINT32: return xyUint32Type;
    case XY_UINT64: return xyUint64Type;
    case XY_FLOAT: return xyFloatType;
    case XY_STRING: return xyStringType;
    case XY_CHAR: return xyCharType;
    case XY_LIST:
        // Must be a function call.
        list = xyTokenGetListVal(token);
        if (xyListGetNumToken(list) == 0) {
            xyError(token, "Empty function call");
        }
        refToken = xyListGetiToken(list, 0);
        if (xyTokenGetType(refToken) != XY_IDREF) {
            xyError(token, "Expected function call");
        }

        break;
    default:
        // Nothing needed.
    }
    return xyIdentNull;
}

// Dot expressions look for identifiers in the scope of the type of the left
// expression.  Find the type, and then bind to an ident in its scope.
static void bindDotExpression(xyToken token, xyIdent parentScope) {
    xyList list = xyTokenGetList(token);
    if (list == xyListNull || xyTokenGetListIndex(token) != 0 ||
            xyListGetNumToken(list) != 3) {
        xyError(token, "Invalid dot expression");
    }
    xyToken exprToken = xyListGetiToken(list, 1);
    xyToken idrefToken = xyListGetiToken(list, 2);
    if (xyTokenGetType(idrefToken) != XY_IDREF) {
        xyError(token, "Must have IDREF token in dot expression");
    }
    xyIdent typeIdent = findType(exprToken, parentScope);
    if (typeIdent == xyIdentNull) {
        xyError(token, "Dot expression on null type");
    }
    xyTokenSetIdent(idrefToken, typeIdent);
}

// Bind identifiers recursively under this ident's scope.
void coBindIdentifiers(xyToken token, xyIdent parentScope) {
    xyTokenType type = xyTokenGetType(token);
    xyList list = xyTokenGetList(token);
    xyIdent ident;
    switch (type) {
    case XY_IDSCOPE: case XY_IDFUNC: case XY_IDTYPE:
        // Must already have been bound.
        utAssert(xyListGetIdent(list) == xyTokenGetIdent(token));
        break;
    case XY_IDREF:
        utAssert(xyTokenGetIdent(token) == xyIdentNull);
        utSym sym = xyTokenGetSymVal(token);
        ident = xyLookupIdent(parentScope, sym);
        if(ident == xyIdentNull) {
            xyError(token, "Identifier %s is not found", utSymGetName(sym));
        }
        xyTokenSetIdent(token, ident);
        break;
    case XY_LIST:
        bindListIdentifiers(xyTokenGetListVal(token), parentScope);
        break;
    case XY_DOT:
        bindDotExpression(token, parentScope);
        break;
    default:
        // Nothing to do.
        break;
    }
}
