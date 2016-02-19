%{

#include "parse_int.h"
#include "ruleparse.h"

#define YY_NEVER_INTERACTIVE 1
#define YY_INPUT(buf,result,max_size) \
    if((result = xpRead((char *) buf, max_size )) < 0 ) \
        YY_FATAL_ERROR( "input in flex scanner failed" );

static int xpRead(char *buf, int maxSize)
{
    int numChars = 0;
    int c;

    do {
        c = getc(xpFile);
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

int xpwrap(void)
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
<INITIAL>\n                     { xpLineNum++; }

<INITIAL>INT                    { return INT; }
<INITIAL>UINT                   { return UINT; }
<INITIAL>FLOAT                  { return FLOAT; }
<INITIAL>DOUBLE                 { return DOUBLE; }
<INITIAL>BOOL                   { return BOOL; }
<INITIAL>STRING                 { return STRING; }
<INITIAL>IDENT                  { return IDENT; }

<INITIAL>\"([^"]|\\.)*\"        { myDebug("KEYWORD %s\n", xptext);
                                  xplval.symVal = utSymCreate(unmunge(xptext));
                                  return KEYWORD; }
<INITIAL>[a-zA-Z0-9_]+          { myDebug("NONTERM %s\n", xptext);
                                  xplval.symVal = utSymCreate(xptext);
                                  return NONTERM; }
<INITIAL>.                      { myDebug("Char '%c'\n", xptext[0]);
                                  return xptext[0]; }