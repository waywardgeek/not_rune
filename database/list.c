#include <stdlib.h>
#include "database.h"

// Print a list.
void xyPrintList(xyList list) {
    putchar('(');
    bool firstTime = true;
    xyToken token;
    xyForeachListToken(list, token) {
        if(!firstTime) {
            putchar(' ');
        }
        firstTime = false;
        xyPrintToken(token);
    } xyEndListToken;
    putchar(')');
}

// Add text to a dynamically resized buffer.
static void addTextToBuffer(char **buf, uint32 *bufSize, uint32 *pos, char *text) {
    uint32 len = strlen(text);
    if(*pos + len >= *bufSize) {
        *bufSize = (*bufSize << 1) + len;
        *buf = realloc(*buf, *bufSize);
    }
    memcpy(*buf + *pos, text, len);
    *pos += len;
}

// Print a list.
char *xyListGetText(xyList list) {
    uint32 bufSize = 1024;
    char *buf = calloc(bufSize, sizeof(char));
    uint32 pos = 0;
    addTextToBuffer(&buf, &bufSize, &pos, "(");
    bool firstTime = true;
    xyToken token;
    xyForeachListToken(list, token) {
        if(!firstTime) {
            addTextToBuffer(&buf, &bufSize, &pos, " ");
        }
        firstTime = false;
        char *text = xyTokenGetText(token);
        addTextToBuffer(&buf, &bufSize, &pos, text);
    } xyEndListToken;
    addTextToBuffer(&buf, &bufSize, &pos, ")");
    char *retVal = utCopyString(buf);
    free(buf);
    return retVal;
}
