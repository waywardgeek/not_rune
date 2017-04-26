#include "database.h"

xyToken xyArrayType, xyBoolType, xyIntType, xyInt8Type, xyInt16Type;
xyToken xyInt32Type, xyInt64Type, xyUintType, xyUint8Type, xyUint16Type;
xyToken xyUint32Type, xyUint64Type, xyFloatType, xyStringType, xyCharType;
xyToken xyTrueToken, xyFalseToken, xyNullToken;

// Register the binding function for the keyword.
xyBindfunc xyRegisterBindfunc(xyParser parser, utSym sym, xyBindfunc bindfunc) {
    xyMtoken mtoken = xyMtokenCreate(parser, XY_KEYWORD, sym);
    xyBindfunc oldBindfunc = xyMtokenGetBindfunc(mtoken);
    xyMtokenSetBindfunc(mtoken, bindfunc);
    return oldBindfunc;
}

// Restore the previous binding function for the keyword.
void xyUnregisterBindfunc(xyParser parser, utSym sym, xyBindfunc oldBindfunc) {
    xyMtoken mtoken = xyMtokenCreate(parser, XY_KEYWORD, sym);
    xyMtokenSetBindfunc(mtoken, oldBindfunc);
}

// Find the type of a non-keyword token.
static xyToken findBuiltinType(xyIdent parentScope, xyToken token) {
    xyIdent ident;
    utSym sym;
    switch (xyTokenGetType(token)) {
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
    case XY_IDREF:
        sym = xyTokenGetSymVal(token);
        ident = xyLookupIdent(parentScope, sym);
        if(ident == xyIdentNull) {
            xyError(token, "Identifier %s is not found", utSymGetName(sym));
        }
        xyTokenSetIdent(token, ident);
        return xyIdentGetToken(ident);
    default:
        // Nothing needed.
        break;
    }
    return xyTokenNull;
}

// Dot expressions look for identifiers in the scope of the type of the left
// expression.  Find the type, and then bind to an ident in its scope.
static xyToken bindDotExpression(xyIdent parentScope, xyToken token) {
    xyList list = xyTokenGetListVal(token);
    if (xyListGetNumToken(list) != 3) {
        xyError(token, "Invalid dot expression");
    }
    xyToken exprToken = xyListGetiToken(list, 1);
    xyToken idrefToken = xyListGetiToken(list, 2);
    if (xyTokenGetType(idrefToken) != XY_IDREF) {
        xyError(token, "Must have IDREF token in dot expression");
    }
    xyToken typeToken;
    if (xyTokenGetType(exprToken) == XY_LIST) {
        typeToken = xyBindToken(parentScope, exprToken);
    } else {
        typeToken = findBuiltinType(parentScope, exprToken);
    }
    if (typeToken == xyTokenNull) {
        xyError(token, "Dot expression on null type");
    }
    xyIdent refScope = xyTokenGetIdent(typeToken);
    if (refScope == xyIdentNull) {
        // TODO: There is a bug here - we have to build the whole ident tree
        // before binding.
        xyError(token, "Dot expression on non-named type");
    }
    utSym sym = xyTokenGetSymVal(idrefToken);
    xyIdent ident = xyLookupIdent(refScope, sym);
    if (ident == xyIdentNull) {
        xyError(token, "%s not defined on %s", utSymGetName(sym),
            xyIdentGetName(refScope));
    }
    xyTokenSetTypeToken(token, typeToken);
    return typeToken;
}

// Bind statements in a statement list.
static xyToken bindStatements(xyIdent currentScope, xyToken token) {
    xyList list = xyTokenGetListVal(token);
    uint32 i;
    for (i = 1; i < xyListGetNumToken(list); i++) {
        xyToken statementToken = xyListGetiToken(list, i);
        xyBindToken(currentScope, statementToken);
    }
    return xyTokenNull;
}

// Bind all the elements of a list.
void xyBindList(xyIdent currentScope, xyList list) {
    xyToken token;
    xyForeachListToken(list, token) {
        xyBindToken(currentScope, token);
    } xyEndListToken;
}

// Bind parameter declarations.
static void bindParamDecls(xyIdent currentScope, xyToken paramDecls) {
    xyList list = xyTokenGetListVal(paramDecls);
    uint32 i;
    for (i = 0; i < xyListGetNumToken(list); i += 2) {
        xyToken typeToken = xyListGetiToken(list, i);
        xyToken idvarToken = xyListGetiToken(list, i + 1);
        xyBindToken(currentScope, typeToken);
        xyTokenSetTypeToken(idvarToken, typeToken);
    }
}

// Bind a function declaration.
static xyToken bindFunction(xyIdent currentScope, xyToken token) {
    xyList list = xyTokenGetListVal(token);
    xyToken typeToken = xyTokenNull;
    uint32 pos = 1;
    if (xyListGetNumToken(list) == 5) {
        typeToken = xyListGetiToken(list, pos);
        pos++;
        xyBindToken(currentScope, typeToken);
    } else if (xyListGetNumToken(list) != 4) {
        xyError(token, "Wrong number of elements for a function list");
    }
    xyToken funcNameToken = xyListGetiToken(list, pos);
    pos++;
    xyToken paramDecls = xyListGetiToken(list, pos);
    pos++;
    xyToken statementList = xyListGetiToken(list, pos);
    xyTokenSetTypeToken(funcNameToken, typeToken);
    bindParamDecls(currentScope, paramDecls);
    xyBindList(currentScope, xyTokenGetListVal(statementList));
    return typeToken;
}

// Register the keyword's binding function.
static void registerKeyword(xyParser parser, char *name, xyBindfunc bindfunc) {
    utSym sym = utSymCreate(name);
    xyRegisterBindfunc(parser, sym, bindfunc);
}

// Register functions for binding built-in keywords.
static void registerBuiltinBindfuncs(xyParser parser) {
    registerKeyword(parser, ".", bindDotExpression);
    registerKeyword(parser, "statements", bindStatements);
}

// Create a token and identifier for the bultin type.
static xyToken buildTypeToken(xyParser parser, char *name) {
    xyIdent globalScope = xyRootGetGlobalIdent(xyTheRoot);
    utSym sym = utSymCreate(name);
    xyIdent ident = xySymIdentCreate(XY_IDTYPE, globalScope, sym);
    xyToken identToken =  xyIdentTokenCreate(parser, XY_IDTYPE, sym, 0);
    xyIdentInsertToken(ident, identToken);
    return identToken;
}

// Create a token and identifier for the bultin constant.
static xyToken buildConstantToken(xyParser parser, xyToken typeToken, char *name) {
    xyIdent globalScope = xyRootGetGlobalIdent(xyTheRoot);
    utSym sym = utSymCreate(name);
    xyIdent ident = xySymIdentCreate(XY_IDVAR, globalScope, sym);
    xyToken identToken =  xyIdentTokenCreate(parser, XY_IDVAR, sym, 0);
    xyIdentInsertToken(ident, identToken);
    xyTokenSetTypeToken(identToken, typeToken);
    return identToken;
}

// Register built-in keywords, types, and constants.
void xyRegisterBuiltins(xyParser parser) {
    registerBuiltinBindfuncs(parser);
    xyIdent globalScope = xyIdentCreate(xyIdentNull, xyTokenNull);
    xyRootSetGlobalIdent(xyTheRoot, globalScope);
    xyArrayType = buildTypeToken(parser, "array");
    xyBoolType = buildTypeToken(parser, "bool");
    xyIntType = buildTypeToken(parser, "int");
    xyInt8Type = buildTypeToken(parser, "int8");
    xyInt16Type = buildTypeToken(parser, "int16");
    xyInt32Type = buildTypeToken(parser, "int32");
    xyInt64Type = buildTypeToken(parser, "int64");
    xyUintType = buildTypeToken(parser, "uint");
    xyUint8Type = buildTypeToken(parser, "uint8");
    xyUint16Type = buildTypeToken(parser, "uint16");
    xyUint32Type = buildTypeToken(parser, "uint32");
    xyUint64Type = buildTypeToken(parser, "uint64");
    xyFloatType = buildTypeToken(parser, "float");
    xyStringType = buildTypeToken(parser, "string");
    xyCharType = buildTypeToken(parser, "char");
    // Constants
    xyTrueToken = buildConstantToken(parser, xyBoolType, "true");
    xyFalseToken = buildConstantToken(parser, xyBoolType, "false");
    xyNullToken = buildConstantToken(parser, xyTokenNull, "null");
}

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
static void setListScope(xyIdent parentScope, xyList list) {
    xyToken token = findScopedIdentToken(list);
    if(token != xyTokenNull) {
        xyIdent childScope = xyIdentCreate(parentScope, token);
        xyListSetIdent(list, childScope);
    }
}

// Bind the token.  If it is a keyword-prefixed list, bind it recursively using
// its bindfunc method.  Otherwise, it is a built-in type that we bind based on
// the type.
xyToken xyBindToken(xyIdent currentScope, xyToken token) {
    if (xyTokenGetType(token) == XY_LIST) {
        xyList list = xyTokenGetListVal(token);
        if (xyListGetNumToken(list) == 0) {
            xyError(token, "Unexpected empty list");
        }
        xyToken keywordToken = xyListGetiToken(list, 0);
        if (xyTokenGetType(keywordToken) != XY_KEYWORD) {
            xyError(keywordToken, "Expected keyword at start of list");
        }
        setListScope(currentScope, list);
        xyIdent childScope = xyListGetIdent(list);
        if (childScope == xyIdentNull) {
            childScope = currentScope;
        }
        xyBindfunc bindfunc = xyMtokenGetBindfunc(xyTokenGetMtoken(keywordToken));
        if (bindfunc == NULL) {
            xyError(token, "No binding function was registered for this keyword.");
        }
        return bindfunc(childScope, token);
    }
    xyToken typeToken = findBuiltinType(currentScope, token);
    xyTokenSetTypeToken(token, typeToken);
    return typeToken;
}
