%{

#include "rune.h"
#include "ruleparse.h"

#define YY_NEVER_INTERACTIVE 1
#define YY_INPUT(buf,result,max_size) \
    if((result = xyRead((char *) buf, max_size )) < 0 ) \
        YY_FATAL_ERROR( "input in flex scanner failed" );

static int xyRead(char *buf, int maxSize)
{
    int numChars = 0;
    int c;

    do {
        c = getc(xyFile);
        if(c != EOF && c != '\r') {
            *buf++ = c;
            numChars++;
        }
    } while(numChars < maxSize && c != EOF);
    return numChars;
}

static char *unmunge(
    char *string)
{
    char *buffer = utMakeString(strlen(string));
    char *p = buffer;

    string++; /* Skip " */
    while(*string != '\0') {
        if(*string == '\\') {
            string++;
        }
        *p++ = *string++;
    }
    *--p = '\0'; /* Overwrite terminating " */
    return buffer;
}

int xywrap(void)
{
    return 1;
}

#ifdef DV_DEBUG
#define myDebug utDebug
#else
#define myDebug noDebug
#endif
#undef YYLMAX
#define YYLMAX 4096

static void noDebug(char *foo, ...) {}

%}

%Start comment

%%
<INITIAL>^[ \t]*"//".*          ;
<INITIAL>[ \t]+                 ;
<INITIAL>\n                     { xyLineNum++; }

<INITIAL>INT                    { return INT; }
<INITIAL>UINT                   { return UINT; }
<INITIAL>FLOAT                  { return FLOAT; }
<INITIAL>DOUBLE                 { return DOUBLE; }
<INITIAL>BOOL                   { return BOOL; }
<INITIAL>STRING                 { return STRING; }
<INITIAL>IDENT                  { return IDENT; }

<INITIAL>\"([^"]|\\.)*\"        { myDebug("KEYWORD %s\n", xytext);
                                  xylval.symVal = utSymCreate(unmunge(xytext));
                                  return KEYWORD; }
<INITIAL>[a-zA-Z0-9_]+          { myDebug("NONTERM %s\n", xytext);
                                  xylval.symVal = utSymCreate(xytext);
                                  return NONTERM; }
<INITIAL>.                      { myDebug("Char '%c'\n", xytext[0]);
                                  return xytext[0]; }
