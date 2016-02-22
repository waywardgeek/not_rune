/* Parse the input tokens.  I switched from flex because input files are in UTF-8,
   and it looks like flex doesn't yet support it.  Check operators  while
   parsing so we keep multi-character operators together.  Otherwize, just split
   tokens at spaces and boundaries between alnum and non-alnum characters. */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "parse_int.h"

static xyParser paCurrentParser;
static uint8 *paLine;
static uint8 *paText;
static size_t paTextSize, paTextPos;
static uint32 paParenDepth, paBracketDepth;
static bool paLastWasNewline;

// Print out an error message and exit.
void paError(paToken token, char *message, ...) {
    char *buff;
    va_list ap;

    va_start(ap, message);
    buff = utVsprintf((char *)message, ap);
    va_end(ap);
    utError("Line %d, token \"%s\": %s", paTokenGetLineNum(token),
            paTokenGetText(token), buff);
}

// Initialize lexer.
void paLexerStart(xyParser parser) {
    paCurrentParser = parser;
    paLine = NULL;
    paTextSize = 256;
    paText = (uint8 *)calloc(paTextSize, sizeof(uint8));
    paParenDepth = 0;
    paBracketDepth = 0;
    paLastWasNewline = true;
}

// Stop the lexer.
void paLexerStop(void) {
}

// Just print the contents of the token
void paPrintToken(paToken token) {
    printf("%-6u ", paTokenGetLineNum(token));
    switch(paTokenGetType(token)) {
    case XY_TOK_INTEGER:
        printf("INTEGER: %llu\n", paTokenGetIntVal(token));
        break;
    case XY_TOK_FLOAT:
        printf("FLOAT: %g\n", paTokenGetFloatVal(token));
        break;
    case XY_TOK_STRING:
        printf("STRING: %s\n", paTokenGetText(token));
        break;
    case XY_TOK_NEWLINE:
        printf("NEWLINE\n");
        break;
    case XY_TOK_CHAR:
        printf("CHAR: %s\n", paTokenGetText(token));
        break;
    case XY_TOK_IDENT:
        printf("IDENT: %s\n", paTokenGetText(token));
        break;
    case XY_TOK_OPERATOR:
        printf("OPERATOR: %s\n", paTokenGetText(token));
        break;
    case XY_TOK_COMMENT:
        printf("COMMENT: %s\n", paTokenGetText(token));
        break;
    case XY_TOK_KEYWORD:
        printf("KEYWORD: %s\n", paTokenGetText(token));
        break;
    case XY_TOK_BEGIN:
        printf("{\n");
        break;
    case XY_TOK_END:
        printf("}\n");
        break;
    default:
        utExit("Unknown token type");
    }
}

// Create a token object.
static inline paToken paTokenCreate(xyMtokenType type, uint8 *text) {
    paToken token = paTokenAlloc();
    paTokenSetType(token, type);
    paTokenSetText(token, text, strlen((char *)text) + 1);
    paTokenSetLineNum(token, paLineNum);
    utSym sym = utSymNull;
    if(type == XY_TOK_KEYWORD) {
        sym = utSymCreate((char *)text);
    }
    xyMtoken mtoken = xyParserFindMtoken(paCurrentParser, type, sym);
    if(mtoken == xyMtokenNull) {
        paError(token, "Unknown token type: %s", text);
    }
    return token;
}

// Create a new integer token.
static inline paToken paIntTokenCreate(uint64 intVal, uint8 *text) {
    paToken token = paTokenCreate(XY_TOK_INTEGER, text);

    paTokenSetIntVal(token, intVal);
    return token;
}

// Create a new float token.
static inline paToken paFloatTokenCreate(double floatVal, uint8 *text) {
    paToken token = paTokenCreate(XY_TOK_FLOAT, text);

    paTokenSetFloatVal(token, floatVal);
    return token;
}

// Create a new operator or keyword token.  Just check to see if the keyword is
// owned by an operator or staterule.
static inline paToken paKeywordTokenCreate(xyMtoken mtoken, uint8 *text) {
    paToken token = paTokenCreate(XY_TOK_KEYWORD, text);
    paMtokenAppendToken(mtoken, token);
    return token;
}

// Skip blanks and control chars other than tab and newline.
static uint8 *skipSpace(uint8 *p) {
    uint8 c = *p;

    while(c <= ' ' && c != '\0' && c != '\n') {
        c = *++p;
    }
    return p;
}

// Add a character to paText from paLine.
static inline void addChar(void) {
    int length = utf8FindLength(*paLine);

    if(paTextPos + length > paTextSize) {
        paTextSize <<= 1;
        paText = (uint8 *)realloc(paText, paTextSize*sizeof(uint8));
    }
    while(length--) {
        paText[paTextPos++] = *paLine++;
    }
}

// Add an ASCII character to paText from paLine.
static inline void addAscii(uint8 c) {
    if(paTextPos >= paTextSize) {
        paTextSize <<= 1;
        paText = (uint8 *)realloc(paText, paTextSize*sizeof(uint8));
    }
    paText[paTextPos++] = c;
}

// Add numChars characters to paText from paLine.
static inline void addChars(int numChars) {
    while(numChars-- != 0) {
        addChar();
    }
}

// Try to parse a comment.
static inline bool readComment(void) {
    uint32 depth = 0;

    if(*paLine != '/' || (paLine[1] != '/' && paLine[1] != '*')) {
        return false;
    }
    paLine++;
    if(*paLine == '/') {
        paLine++;
        while(*paLine != '\0') {
            addChar();
        }
    } else {
        depth++;
        while(depth != 0) {
            if(*paLine == '\0') {
                addAscii('\n');
                paLine = utf8ReadLine(paFile);
            } else if(*paLine == '*' && paLine[1] == '/') {
                paLine += 2;
                depth--;
            } else {
                addChar();
            }
        }
    }
    addAscii('\0');
    return true;
}

// Try to read an integer, but if a parsed float is longer, do that.
static inline paToken readNumber(void) {
    uint8 c = *paLine;
    char *floatTail, *intTail;
    double floatVal;
    uint64 intVal;

    if(!isdigit(c) && c != '.') {
        return 0;
    }
    floatVal = strtod((char *)paLine, &floatTail);
    if(floatTail == (char *)paLine) {
        return 0;
    }
    intVal = strtoll((char *)paLine, &intTail, 0);
    if(intTail >= floatTail) {
        addChars(intTail - (char *)paLine);
        addAscii('\0');
        return paIntTokenCreate(intVal, paText);
    }
    addChars(floatTail - (char *)paLine);
    addAscii('\0');
    return paFloatTokenCreate(floatVal, paText);
}

// Try to read a string.
static inline bool readString(void) {
    if(*paLine != '"') {
        return false;
    }
    paLine++; // Skip " character
    while(*paLine != '\0' && *paLine != '"') {
        if(*paLine == '\\') {
            paLine++;
            if(*paLine == 'n') {
                addAscii('\n');
                paLine++;
            } else if(*paLine == 'r') {
                addAscii('\r');
                paLine++;
            } else if(*paLine == 't') {
                addAscii('\t');
                paLine++;
            } else {
                addChar();
            }
        } else {
            addChar();
        }
    }
    if(*paLine != '"') {
        utError("Invalid string termination");
    }
    paLine++;
    addAscii('\0');
    return true;
}

// Try to read an operator.  We check up to 4-character long strings of ASCII
// punctuation characters, and accept the longest found.
static inline paToken readOperator(void) {
    xyMtoken mtoken;
    uint8 opString[5];
    int length;

    for(length = 0; length < 4 && ispunct(paLine[length]); length++) {
        opString[length] = paLine[length];
    }
    // Now check in the operator hash table for each size > 0, starting at
    // length.
    while(length) {
        opString[length] = '\0';
        mtoken = xyParserFindMtoken(paCurrentParser, XY_TOK_KEYWORD, utSymCreate((char *)opString));
        if(mtoken != xyMtokenNull) {
            paLine += length;
            return paKeywordTokenCreate(mtoken, opString);
        }
        length--;
    }
    return paTokenNull;
}

// Try to read a keyword.
static inline paToken lookForKeyword(void) {
    xyMtoken mtoken = xyParserFindMtoken(paCurrentParser, XY_TOK_KEYWORD, utSymCreate((char *)paText));

    if(mtoken == xyMtokenNull) {
        return paTokenNull;
    }
    return paKeywordTokenCreate(mtoken, paText);
}

// Read an identifier.  This should work so long as the first character is alpha
// numeric or is not a plain ASCII character (has it's high bit set).
static inline bool readIdentifier(void) {
    uint8 c = *paLine;

    if(!(c & 0x80) && !isalnum(c) && c != '\\') {
        return false;
    }
    while((c & 0x80) || isalnum(c) || c == '\\') {
        addChar();
        c = *paLine;
    }
    addAscii('\0');
    return true;
}

// Read one token, now that we know we've got some text to parse.
static paToken readToken(void) {
    paToken token;

    if(readComment()){
        return paTokenCreate(XY_TOK_COMMENT, paText);
    }
    if(readString()) {
        return paTokenCreate(XY_TOK_STRING, paText);
    }
    token = readNumber();
    if(token != paTokenNull) {
        return token;
    }
    token = readOperator();
    if(token != paTokenNull) {
        return token;
    }
    if(readIdentifier()) {
        token = lookForKeyword();
        if(token != paTokenNull) {
            return token;
        }
        return paTokenCreate(XY_TOK_IDENT, paText);
    }
    // Must just be a single punctuation character
    addChar();
    addAscii('\0');
    return paTokenCreate(XY_TOK_CHAR, paText);
}

// Determine if paLine is nothing but spaces, tabs, and a single backslash.
static inline bool lineIsSlash(void) {
    bool hasBackslash = false;
    uint8 *p = paLine;
    uint8 c;

    while((c = *p++) != '\0') {
        if(c == '\\') {
            if(hasBackslash) {
                return false;
            }
            hasBackslash = true;
        } else if(c != ' ' && c != '\t') {
            return false;
        }
    }
    return hasBackslash;
}

// Parse one token.
static paToken lexRawToken(void) {
    paTextPos = 0;
    if(paLine == NULL) {
        paLine = utf8ReadLine(paFile);
        if(paLine == NULL) {
            return paTokenNull;
        }
        paLineNum++;
    }
    while(lineIsSlash()) {
        paLine = utf8ReadLine(paFile);
        if(paLine == NULL) {
            return paTokenNull;
        }
        paLineNum++;
    }
    paLine = skipSpace(paLine);
    if(*paLine == '\0') {
        paLine = NULL;
        return paTokenCreate(XY_TOK_NEWLINE, (uint8 *)"\n");
    }
    return readToken();
}

// Determine if the input line is blank.
static inline bool lineIsBlank(void) {
    uint8 *p;
    uint8 c;

    if(lineIsSlash()) {
        return true;
    }
    p = paLine;
    while((c = *p++) != '\0') {
        if(c != ' ' && c != '\t') {
            return false;
        }
    }
    return true;
}

// Skip blank lines in the input.
static void skipBlankLines(void) {
    if(paLine != NULL) {
        return; // Nothing to skip.
    }
    paLine = utf8ReadLine(paFile);
    while(paLine != NULL && lineIsBlank()) {
        paLineNum++;
        paLine = utf8ReadLine(paFile);
    }
    if(paLine != NULL) {
        paLineNum++;
    }
}

// Parse a single token.
paToken paLex(void) {
    paToken token;
    xyMtokenType type;
    char *text;

    if(paLastWasNewline) {
        skipBlankLines();
    }
    token = lexRawToken();
    if(token == paTokenNull) {
        return token;
    }
    type = paTokenGetType(token);
    // Eat newlines inside grouping operators
    while(type == XY_TOK_NEWLINE && (paParenDepth > 0 || paBracketDepth > 0)) {
        paTokenDestroy(token);
        token = lexRawToken();
        type = paTokenGetType(token);
    }
    // TODO: Deal with eating newlines between keywords and grouping operators, rather
    // than just these.
    text = (char *)paTokenGetText(token);
    if(type == XY_TOK_OPERATOR) {
        if(!strcmp(text, "(")) {
            paParenDepth++;
        } else if(!strcmp(text, "[")) {
            paBracketDepth++;
        } else if(!strcmp(text, ")")) {
            paParenDepth--;
        } else if(!strcmp(text, "]")) {
            paBracketDepth--;
        }
    }
    if(type == XY_TOK_CHAR) {
        if(!strcmp(text, "{")) {
            paTokenDestroy(token);
            token = paTokenCreate(XY_TOK_BEGIN, (uint8 *)"{");
            skipBlankLines();
        } else if(!strcmp(text, "}")) {
            paTokenDestroy(token);
            token = paTokenCreate(XY_TOK_END, (uint8 *)"}");
            skipBlankLines();
        }
    }
    paLastWasNewline = type == XY_TOK_NEWLINE;
    return token;
}
