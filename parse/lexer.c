/* Parse the input tokens.  I switched from flex because input files are in UTF-8,
   and it looks like flex doesn't yet support it.  Check operators  while
   parsing so we keep multi-character operators together.  Otherwize, just split
   tokens at spaces and boundaries between alnum and non-alnum characters. */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "parse_int.h"

static uint8 *paLine;
static uint8 *paText;
static size_t paTextSize, paTextPos;
static bool paLastWasNewline;
static bool paEnding;
static uint32 paParenDepth, paBracketDepth;

// Print out an error message and exit.
void paError(xyToken token, char *message, ...) {
    char *buff;
    va_list ap;
    va_start(ap, message);
    buff = utVsprintf((char *)message, ap);
    va_end(ap);
    utError("Line %d, token \"%s\": %s", xyTokenGetLinenum(token),
            xyTokenGetText(token), buff);
}

// Initialize lexer.
void paLexerStart() {
    paLine = NULL;
    paTextSize = 256;
    paText = (uint8 *)calloc(paTextSize, sizeof(uint8));
    paLastWasNewline = true;
    paEnding = false;
    paParenDepth = 0;
    paBracketDepth = 0;
}

// Stop the lexer.
void paLexerStop(void) {
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
static inline xyToken readNumber(void) {
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
        return xyIntTokenCreate(paCurrentParser, intVal, paLinenum);
    }
    addChars(floatTail - (char *)paLine);
    addAscii('\0');
    return xyFloatTokenCreate(paCurrentParser, floatVal, paLinenum);
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
static inline xyToken readOperator(void) {
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
        mtoken = xyParserFindMtoken(paCurrentParser, XY_KEYWORD, utSymCreate((char *)opString));
        if(mtoken != xyMtokenNull) {
            paLine += length;
            return xyKeywordTokenCreate(paCurrentParser, utSymCreate((char *)opString), paLinenum);
        }
        length--;
    }
    return xyTokenNull;
}

// Try to read a keyword.
static inline xyToken lookForKeyword(void) {
    xyMtoken mtoken = xyParserFindMtoken(paCurrentParser, XY_KEYWORD, utSymCreate((char *)paText));
    if(mtoken == xyMtokenNull) {
        return xyTokenNull;
    }
    return xyKeywordTokenCreate(paCurrentParser, utSymCreate((char *)paText), paLinenum);
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
static xyToken readToken(void) {
    xyToken token;
    while(readComment());
    if(readString()) {
        return xyStringTokenCreate(paCurrentParser, paText, paLinenum);
    }
    token = readNumber();
    if(token != xyTokenNull) {
        return token;
    }
    token = readOperator();
    if(token != xyTokenNull) {
        return token;
    }
    if(readIdentifier()) {
        token = lookForKeyword();
        if(token != xyTokenNull) {
            return token;
        }
        return xyIdentTokenCreate(paCurrentParser, utSymCreate((char *)paText), paLinenum);
    }
    // Must just be a single punctuation character
    addChar();
    addAscii('\0');
    return xyCharTokenCreate(paCurrentParser, (char *)paText, paLinenum);
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
static xyToken lexRawToken(void) {
    paTextPos = 0;
    if(paLine == NULL) {
        paLine = utf8ReadLine(paFile);
        if(paLine == NULL) {
            return xyEOFTokenCreate(paCurrentParser, paLinenum);
        }
        paLinenum++;
    }
    while(lineIsSlash()) {
        paLine = utf8ReadLine(paFile);
        if(paLine == NULL) {
            return xyEOFTokenCreate(paCurrentParser, paLinenum);
        }
        paLinenum++;
    }
    paLine = skipSpace(paLine);
    if(*paLine == '\0') {
        paLine = NULL;
        return xyNewlineTokenCreate(paCurrentParser, paLinenum);
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
        paLinenum++;
        paLine = utf8ReadLine(paFile);
    }
    if(paLine != NULL) {
        paLinenum++;
    }
}

// Parse a single token.
xyToken paLex(void) {
    if(paEnding) {
        return xyEOFTokenCreate(paCurrentParser, paLinenum);
    }
    bool hadNewline = paLastWasNewline;
    xyToken token;
    xyTokenType type;
    if(paLastWasNewline) {
        skipBlankLines();
    }
    token = lexRawToken();
    type = xyTokenGetType(token);
    // Eat newlines inside grouping operators
    while(type == XY_NEWLINE && (paParenDepth > 0 || paBracketDepth > 0)) {
        xyTokenDestroy(token);
        token = lexRawToken();
        type = xyTokenGetType(token);
    }
    char *text = (char *)xyTokenGetText(token);
    if(type == XY_KEYWORD) {
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
    // Add newline to end of file if missing
    if(type == XY_EOF && !hadNewline) {
        paEnding = true;
        return xyNewlineTokenCreate(paCurrentParser, paLinenum);
    }
    paLastWasNewline = type == XY_NEWLINE;
    return token;
}
