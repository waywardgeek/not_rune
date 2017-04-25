#include "core_int.h"

// Find any scoped identifier in the list.  It is illegal to have more than
// one, or to have one in a scoped list since scoped lists are anonymous
// namespaces.
static xyToken findScopedIdentToken(xyList list) {
    xyToken token;
    xyToken scopedToken = xyTokenNull;
    xyForeachListToken(list, token) {
        if(xyTokenGetType(token) == XY_IDSCOPE) {
            if(scopedToken != xyTokenNull) {
                xyError(token, "List contains two scoped identifiers");
            }
            scopedToken = token;
        }
    } xyEndListToken;
    return scopedToken;
}

// Determine if the list is a dotted path.  All tokens in the list must be
// XY_IDDOT tokens.  If they mix types, it is an error.
static bool listIsDottedPath(xyList list) {
    if(xyListGetUsedToken(list) == 0) {
        return false;
    }
    xyToken token = xyListGetiToken(list, 0);
    if(xyTokenGetType(token) == XY_DOT) {
        return true;
    }
    return false;
}

// Bind a dotted path.
static void bindDottedPath(xyList list, xyIdent parentScope) {
    xyToken token;
    xyForeachListToken(list, token) {
        /*
        if(xyTokenGetType(token) != XY_DOT) {
            xyError(token, "Non-dotted token in dotted path - check syntax rules");
        }
        utAssert(xyTokenGetIdent(token) == xyIdentNull);
        utSym sym = xyTokenGetSymVal(token);
        xyIdent ident = xyLookupIdent(parentScope, sym);
        if(ident == xyIdentNull) {
            xyError(token, "Identifier %s is not defined", utSymGetName(sym));
        }
        xyIdent ident = xyIdentCreate(ident);
        xyTokenSetIdref(token, idref);
        parentScope = ident;
        */
    } xyEndListToken;
}

// Bind identifiers recursively under this ident's scope.
static void bindListIdentifiers(xyList list, xyIdent parentScope) {
    /*
    if(listIsDottedPath(list)) {
        bindDottedPath(list, parentScope);
        return;
    }
    xyToken token = findScopedIdentToken(list);
    if(token != xyTokenNull) {
        if(xyListScoped(list)) {
            xyError(token, "Anonymous scoped list has a scoped identifier");
        }
        utAssert(xyTokenGetIdent(token) == xyIdentNull);
        parentScope = xyIdentCreate(parentScope, xyTokenGetSymVal(token));
        xyTokenSetIdent(token, parentScope);
        xyListSetIdent(list, parentScope);
        xyListSetScoped(list, true);
    } else if(xyListScoped(list)) {
        // Create an anonymous scope.
        parentScope = xyIdentCreate(parentScope, utSymNull);
        xyListSetIdent(list, parentScope);
    }
    xyForeachListToken(list, token) {
        coBindIdentifiers(token, parentScope);
    } xyEndListToken;
    */
}

// Find the type of an expression.  In the core syntax, all expressions are
// either literals or function calls.  If it is a function call, then the type
// is just the function's type expression, or null if it has no type.  If it is
// a literal, the type is the type of the literal, which is represeted by the
// literal token itself.
static xyIdent findType(xyToken token, xyIdent parentScope) {
    /*
    xyList list;
    xyToken refToken;
    switch (xyTokenGetType(token)) {
    case XY_KEYWORD: return xyIdentNull;
    case XY_INT: return xyIntType;
    case BOOL: return xyBoolType;
    case FLOAT: return xyFloatType;
    case STRING: return xyStringType;
    case CHAR: return xyCharType;
    case LIST:
        // Must be a function call.
        list = xyTokenGetListVal(token);
        if (xyListGetNumToken(list) < 1) {
            xyError(token, "Empty function call");
        }
        ...
        refToken = xyListGetiToken(list, 0);
        funcSig = findFuncWithSig(convertToSig(
    default:
        // Nothing needed.
    }
    */
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
    xyIdent typeIdent = findType(token, parentScope);
    if (typeIdent == xyIdentNull) {
        xyError(token, "Dot expression on null type");
    }
    xyTokenSetIdent(idrefToken, typeIdent);
}

// Bind identifiers recursively under this ident's scope.
void coBindIdentifiers(xyToken token, xyIdent parentScope) {
    xyTokenType type = xyTokenGetType(token);
    switch (type) {
    case XY_IDSCOPE: case XY_IDFUNC: case XY_IDTYPE: case XY_IDVAR:
        utAssert(xyTokenGetIdent(token) == xyIdentNull);
        xyIdent ident = xyIdentCreate(parentScope, token);
        break;
    case XY_IDREF:
        utAssert(xyTokenGetIdent(token) == xyIdentNull);
        utSym sym = xyTokenGetSymVal(token);
        ident = xyLookupIdent(parentScope, sym);
        if(ident == xyIdentNull) {
            xyError(token, "Identifier %s is not defined", utSymGetName(sym));
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
