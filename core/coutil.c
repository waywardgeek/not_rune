#include <ctype.h>
#include "core_int.h"

// Find out how many args are used in a template.
static uint32 countArgs(char *temp) {
    char c;
    uint32 maxArg = 0, xArg;
    while (*temp) {
        c = *temp++;
        if (c == '%') {
            c = *temp;
            if (c == 'l' || c == 'u' || c == 'c') {
                temp++;
                c = *temp;
            }
            if(isdigit((unsigned char)c)) {
                temp++;
                xArg = c - '0';
                if (xArg >= maxArg) {
                    maxArg = xArg + 1;
                }
            } else if (c == '%') {
                temp++;
            }
        }
    }
    return maxArg;
}

// This code manages a simple string buffer.
static char *prStringBuffer;
static uint32 prStringBufferSize;
static uint32 prStringBufferPosition;

// Append a string to the string buffer.
static void appendString(char *string) {
    uint32 length = strlen(string);
    if(prStringBufferPosition + length + 1 >= prStringBufferSize) {
        prStringBufferSize = ((prStringBufferPosition + length + 1)*3) >> 1;
        utResizeArray(prStringBuffer, prStringBufferSize);
    }
    strcpy(prStringBuffer + prStringBufferPosition, string);
    prStringBufferPosition += length;
}

// Append a character to the string buffer.
static void appendChar(char c) {
    char string[2];
    string[0] = c;
    string[1] = '\0';
    appendString(string);
}

// Initialize the utility module, allocating buffers.
void coUtilStart(void)
{
    prStringBufferSize = 42;
    prStringBuffer = utNewA(char, prStringBufferSize);
    prStringBufferPosition = 0;
}

// Free memory used by the utility module.
void coUtilStop(void) {
    utFree(prStringBuffer);
}

// Write a template to a string.
static void wrtemp(char *temp, va_list ap) {
    uint32 sArg = countArgs(temp), xArg;
    char *(args[10]);
    char *string, *arg;
    char c;
    bool lowerCase = false, upperCase = false, caps = false;
    prStringBufferPosition = 0;
    prStringBuffer[0] = '\0';
    for (xArg = 0; xArg < sArg; xArg++) {
        args[xArg] = va_arg(ap, char *);
    }
    string = temp;
    while (*string) {
        c = *string++;
        if (c == '%') {
            c = *string;
            if(c == 'l') {
                lowerCase = true;
                c = *++string;
            } else if(c == 'u') {
                upperCase = true;
                c = *++string;
            } else if(c == 'c') {
                caps = true;
                c = *++string;
            }
            if(isdigit((unsigned char)c)) {
                string++;
                xArg = c - '0';
                if(xArg >= sArg) {
                    utExit("exWrtemp: not enough args");
                }
                if (*args[xArg]) {
                    if(lowerCase) {
                        appendChar((char)tolower((unsigned char)*(args[xArg])));
                        appendString((args[xArg]) + 1);
                        lowerCase = false;
                    } else if(upperCase) {
                        appendChar((char)toupper((unsigned char)*(args[xArg])));
                        appendString((args[xArg]) + 1);
                        upperCase = false;
                    } else if(caps) {
                        arg = args[xArg];
                        while(*arg) {
                            appendChar((char)toupper((unsigned char)*arg));
                            arg++;
                        }
                        caps = false;
                    } else {
                        appendString(args[xArg]);
                    }
                }
            } else if (c == '%') {
                string++;
                appendChar('%');
            }
        } else {
            appendChar(c);
        }
    }
}

// Write a template to a string.
char *coSwrtemp(char *temp, ...) {
    va_list ap;

    va_start(ap, temp);
    wrtemp(temp, ap);
    va_end(ap);
    return utCopyString(prStringBuffer);
}

// Write a template to a file.
void coWrtemp(FILE *file, char *temp, ...) {
    va_list ap;
    va_start(ap, temp);
    wrtemp(temp, ap);
    va_end(ap);
    fputs(prStringBuffer, file);
}

// Return a symbol with the same name, but the first letter capitalized.
utSym coUpperSym(utSym sym) {
    char *name;

    if(sym == utSymNull) {
        return utSymNull;
    }
    name = utCopyString(utSymGetName(sym));
    *name = toupper((unsigned char)*name);
    return utSymCreate(name);
}
