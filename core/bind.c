#include "core_int.h"

// Find any scoped identifier in the list.  It is illegal to have more than
// one, or to have one in a scoped list since scoped lists are anonymous
// namespaces.
static xyToken findScopedIdentToken(xyList list) {
    xyToken token;
    xyToken scopedToken = xyTokenNull;
    xyForeachListToken(list, token) {
        if(xyTokenGetType(token) == XY_IDSCOPED) {
            if(scopedToken != xyTokenNull) {
                xyError(token, "List contains two scoped identifiers");
            }
            scopedToken = token;
        }
    } xyEndListToken;
    return scopedToken;
}

// Bind identifiers recursively under this ident's scope.
static void bindListIdentifiers(xyList list, xyIdent parentScope) {
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
}

// Bind identifiers recursively under this ident's scope.
void coBindIdentifiers(xyToken token, xyIdent parentScope) {
    xyTokenType type = xyTokenGetType(token);
    if(type == XY_IDENT) {
        xyError(token, "Identifier has not been marked as def, ref, or scoped");
    } else if(type == XY_IDDEF) {
        utAssert(xyTokenGetIdent(token) == xyIdentNull);
        xyIdent ident = xyIdentCreate(parentScope, xyTokenGetSymVal(token));
        xyTokenSetIdent(token, ident);
    } else if(type == XY_IDREF) {
        utAssert(xyTokenGetIdref(token) == xyIdrefNull);
        utSym sym = xyTokenGetSymVal(token);
        xyIdent ident = xyLookupIdent(parentScope, sym);
        if(ident == xyIdentNull) {
            xyError(token, "Identifier %s is not defined", utSymGetName(sym));
        }
        xyIdref idref = xyIdrefCreate(ident);
        xyTokenSetIdref(token, idref);
    } else if(type == XY_IDDOT) {
        // TODO: Deal with dotted paths when we do type propagation
        // I don't recall why I had that TODO.  Isn't this just the same as the XY_IDREF case?
    } else if(type == XY_IDSCOPED) {
    } else if(type == XY_LIST) {
        bindListIdentifiers(xyTokenGetListVal(token), parentScope);
    }
}
