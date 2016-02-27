#include "core_int.h"

// Bind identifiers recursively under this ident's scope.
static void bindListIdentifiers(xyList list, xyIdent ident) {
    xyToken token;
    xyForeachListToken(list, token) {
        coBindIdentifiers(token, ident);
        if(xyListGetIdent(list) != xyIdentNull) {
            // This happens when hitting scopped identifiers
            ident = xyListGetIdent(list);
        }
    } xyEndListToken;
    if(xyListScoped(list) && xyListGetIdent(list) == xyIdentNull) {
        // This is an unnamed anonymous scope.
        xyIdent unnamedIdent = xyIdentCreate(ident, utSymNull);
        xyListSetIdent(list, unnamedIdent);
    }
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
        // TODO: deal with paths.
        utAssert(xyTokenGetIdref(token) == xyIdrefNull);
        utSym sym = xyTokenGetSymVal(token);
        xyIdent ident = xyLookupIdent(parentScope, sym);
        if(ident == xyIdentNull) {
            xyError(token, "Identifier %s is not defined", utSymGetName(sym));
        }
        xyIdref idref = xyIdrefCreate(ident);
        xyTokenSetIdref(token, idref);
    } else if(type == XY_IDSCOPED) {
        utAssert(xyTokenGetIdent(token) == xyIdentNull);
        xyIdent ident = xyIdentCreate(parentScope, xyTokenGetSymVal(token));
        xyTokenSetIdent(token, ident);
        xyList list = xyTokenGetList(token);
        utAssert(list != xyListNull);
        xyListSetScoped(list, true);
        xyListSetIdent(list, ident);
    } else if(type == XY_LIST) {
        bindListIdentifiers(xyTokenGetListVal(token), parentScope);
    }
}
