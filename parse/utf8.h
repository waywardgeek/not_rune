// utf8 module

typedef unsigned char uchar;

void utf8Start(void);
void utf8Stop(void);
uchar *utf8ReadLine(FILE *file);
static inline int utf8FindLength(uchar c) {
    int expectedLength = 1;
    while(c & 0x80) {
        expectedLength++;
        c <<= 1;
    }
    return expectedLength;
}

