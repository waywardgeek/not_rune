#include "database.h"

// Print out an error message and exit.
void xyError(xyToken token, char *message, ...) {
    char *buff;
    va_list ap;
    va_start(ap, message);
    buff = utVsprintf((char *)message, ap);
    va_end(ap);
    utError("Line %d, token \"%s\": %s", xyTokenGetLinenum(token),
            xyTokenGetText(token), buff);
}

// Get the name of an xyTokenType.
char *xyTokenTypeGetName(xyTokenType type) {
    switch(type) {
    case XY_NONTERM: return "NONTERM";
    case XY_KEYWORD: return "KEYWORD";
    case XY_INT: return "INT";
    case XY_FLOAT: return "FLOAT";
    case XY_BOOL: return "BOOL";
    case XY_STRING: return "STRING";
    case XY_LIST: return "LIST";
    case XY_CHAR: return "CHAR";
    case XY_DOT: return "DOT";
    case XY_IDENT: return "IDENT";
    case XY_IDREF: return "IDREF";
    case XY_IDSCOPE: return "IDSCOPE";
    case XY_IDFUNC: return "IDFUNC";
    case XY_IDTYPE: return "IDTYPE";
    case XY_IDVAR: return "IDVAR";
    case XY_NEWLINE: return "NEWLINE";
    case XY_EOF: return "EOF";
    case XY_NULL: return "null";
    default:
        utExit("Unknown token type");
    }
    return NULL; // Dummy return
}

// Return text representing the token.
char *xyTokenGetText(xyToken token) {
    switch(xyTokenGetType(token)) {
    case XY_INT:
        return utSprintf("%lld", xyTokenGetIntVal(token));
    case XY_FLOAT:
        return utSprintf("%f", xyTokenGetFloatVal(token));
    case XY_BOOL:
        return utSprintf("%s", xyTokenBoolVal(token)? "true" : "false");
    case XY_STRING:
        return utSprintf("%s", xyStringGetText(xyTokenGetStringVal(token)));
    case XY_LIST:
        return xyListGetText(xyTokenGetListVal(token));
    case XY_DOT: return ".";
    case XY_IDENT:
    case XY_IDREF:
    case XY_IDSCOPE:
    case XY_IDFUNC:
    case XY_IDTYPE:
    case XY_IDVAR:
        return utSprintf("%s", utSymGetName(xyTokenGetSymVal(token)));
    case XY_NONTERM:
    case XY_KEYWORD:
        return utSprintf("%s", xyMtokenGetName(xyTokenGetMtoken(token)));
    case XY_CHAR:
    {
        uint32 charVal = xyTokenGetCharVal(token);
        if(charVal <= '~' && charVal >= ' ') {
            return utSprintf("'%c'", charVal);
        }
        return utSprintf("unicode(0x%x)", charVal);
    }
    case XY_NEWLINE:
        return utSprintf("NEWLINE");
    case XY_EOF:
        return utSprintf("EOF");
    case XY_NULL:
        return utSprintf("null");
    default:
        utExit("Unknown token type");
    }
    return NULL; // Dummy return
}

// Print the token.
void xyPrintToken(xyToken token) {
    printf("%s", xyTokenGetText(token));
}

static inline xyToken tokenCreate(xyParser parser, xyTokenType type, utSym sym, uint32 linenum) {
    xyToken token = xyTokenAlloc();
    xyTokenSetType(token, type);
    xyTokenSetLinenum(token, linenum);
    xyMtoken mtoken = xyMtokenCreate(parser, type, sym);
    xyTokenSetMtoken(token, mtoken);
    return token;
}

// Create a new sym token.
xyToken xyIdentTokenCreate(xyParser parser, xyTokenType type, utSym sym, uint32 linenum) {
    xyToken token = tokenCreate(parser, type, utSymNull, linenum);
    xyTokenSetSymVal(token, sym);
    return token;
}

// Create a new int token.
xyToken xyIntTokenCreate(xyParser parser, int64 intVal, uint32 linenum) {
    xyToken token = tokenCreate(parser, XY_INT, utSymNull, linenum);
    xyTokenSetIntVal(token, intVal);
    return token;
}

// Create a new float token.
xyToken xyFloatTokenCreate(xyParser parser, double floatVal, uint32 linenum) {
    xyToken token = tokenCreate(parser, XY_FLOAT, utSymNull, linenum);
    xyTokenSetFloatVal(token, floatVal);
    return token;
}

// Create a new string token.
xyToken xyStringTokenCreate(xyParser parser, uint8 *stringVal, uint32 linenum) {
    xyToken token = tokenCreate(parser, XY_STRING, utSymNull, linenum);
    xyString string = xyStringCreate(stringVal);
    xyTokenSetStringVal(token, string);
    return token;
}

// Create a new list token.
xyToken xyListTokenCreate(xyParser parser, xyList list, uint32 linenum) {
    xyToken token = tokenCreate(parser, XY_LIST, utSymNull, linenum);
    xyTokenSetListVal(token, list);
    return token;
}

// Create a new bool token.
xyToken xyBoolTokenCreate(xyParser parser, bool boolVal, uint32 linenum) {
    xyToken token = tokenCreate(parser, XY_BOOL, utSymNull, linenum);
    xyTokenSetBoolVal(token, boolVal);
    return token;
}

// Create a new keyword token.
xyToken xyKeywordTokenCreate(xyParser parser, utSym sym, uint32 linenum) {
    return tokenCreate(parser, XY_KEYWORD, sym, linenum);
}

// Create a new EOF token.
xyToken xyEOFTokenCreate(xyParser parser, uint32 linenum) {
    return tokenCreate(parser, XY_EOF, utSymNull, linenum);
}

// Create a new NEWLINE token.
xyToken xyNewlineTokenCreate(xyParser parser, uint32 linenum) {
    return tokenCreate(parser, XY_NEWLINE, utSymNull, linenum);
}

// Create a new null token.
xyToken xyNullTokenCreate(xyParser parser, uint32 linenum) {
    return tokenCreate(parser, XY_NULL, utSymNull, linenum);
}

// Convert 4 bytes to a Uint32 big-endian.
static uint32 strToUint32(char *text) {
    uint32 value = 0;
    char c;
    while((c = *text++)) {
        value <<= 8;
        value |= c;
    }
    return value;
}

// Create a new char token.  It is encoded in UTF8 in 4 chars.
xyToken xyCharTokenCreate(xyParser parser, char *text, uint32 linenum) {
    xyToken token = tokenCreate(parser, XY_CHAR, utSymNull, linenum);
    utAssert(strlen(text) <= 4);
    uint32 charVal = strToUint32(text);
    xyTokenSetCharVal(token, charVal);
    return token;
}

// Find any scoped identifier in the list.  It is illegal to have more than
// one.
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
