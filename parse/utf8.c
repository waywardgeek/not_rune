#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "ddutil.h"

static uint8 *line;
static size_t lineSize;
static FILE *currentFile;

// Initialize memory used in the UTF-8 reader
void utf8Start(void)
{
    lineSize = 256;
    line = (uint8 *)calloc(lineSize, sizeof(uint8));
}

// Free memory used by the UTF-8 reader
void utf8Stop(void)
{
    free(line);
    lineSize = 0;
}

// Return the length of the UTF-8 character pointed to by p.  Check that the
// encoding seems valid. We do the full check as defined on Wikipedia.
int utf8FindLengthAndValidate(uint8 *p, bool *valid)
{
    int length, expectedLength, bits;
    unsigned long unicodeCharacter;
    uint8 c = *p;

    *valid = true;
    if((c & 0x80) == 0) {
        // It's ASCII
        if(c < ' ' && c != '\t') {
            // Remove control characters other than tab
            *valid = false;
        }
        return 1;
    }
    c <<= 1;
    expectedLength = 1;
    while(c & 0x80) {
        expectedLength++;
        c <<= 1;
    }
    unicodeCharacter = c >> expectedLength;
    bits = 7 - expectedLength;
    if(expectedLength > 4 || expectedLength == 1) {
        // No unicode values are coded for more than 4 bytes 
        *valid = false;
    }
    if(expectedLength == 1 || (expectedLength == 2 && unicodeCharacter <= 1)) {
        // We could have coded this as ASCII 
        *valid = false;
    }
    length = 1;
    c = *++p;
    while((c & 0xc0) == 0x80) {
        unicodeCharacter = (unicodeCharacter << 6) | (c & 0x3f);
        bits += 6;
        length++;
        c = *++p;
    }
    if(length != expectedLength || unicodeCharacter > 0x10ffff ||
        (unicodeCharacter >= 0xd800 && unicodeCharacter <= 0xdfff)) {
        /* Unicode only defines characters up to 0x10ffff, and excludes values
           0xd800 through 0xdfff */
        *valid = false;
    }
    /* Check to see if we could have encoded the character in the next smaller
       number of bits, in which case it's invalid. */
    if(unicodeCharacter >> (bits - 5) == 0) {
        *valid = false;
    }
    return length;
}

// Make sure that only valid UTF-8 characters are in the line, and that all
// control characters are gone.
static void validateLine(void)
{
    uint8 *p = line;
    uint8 *q = line;
    int length;
    bool valid;

    while(*p != '\0') {
        length = utf8FindLengthAndValidate(p, &valid);
        if(valid) {
            while(length--) {
                *q++ = *p++;
            }
        } else {
            p += length;
        }
    }
    *q = '\0';
}

static inline int readChar(void)
{
    if(currentFile != NULL) {
        return getc(currentFile);
    }
    return getchar();
}

// Check that we have enough room in the line buffer, and resize if needed.
static inline void checkLineSize(
    int pos)
{
    if(pos == lineSize) {
        lineSize <<= 1;
        line = (uint8 *)realloc(line, lineSize*sizeof(uint8));
        if(line == NULL) {
            utExit("Out of memory reading line");
        }
    }
}

// Read a line.  If it's longer than some outragiously long ammount, truncate it. 
static bool readLineRaw(void)
{
    int c = readChar();
    int pos = 0;

    if(c == EOF) {
        return false;
    }
    while(c != '\n') {
        checkLineSize(pos);
        line[pos++] = c;
        c = readChar();
        if(c == EOF) {
            return false;
        }
    }
    checkLineSize(pos);
    line[pos] = '\0';
    return true;
}

// Read a line and validate it, removing control characters and invalid UTF-8 characters.
uint8 *utf8ReadLine(
    FILE *file)
{
    currentFile = file;
    if(!readLineRaw()) {
        return NULL;
    }
    validateLine();
    return line;
}
