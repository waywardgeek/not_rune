%{

#include "parsegen_int.h"
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

#ifdef XP_DEBUG
#define myDebug printf
#define myToken(token) {printf("%s\n", xptext); return (token);}
#else
#define myDebug noDebug
#define myToken(token) return (token)
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

<INITIAL>INTEGER                { myToken(KWINTEGER); }
<INITIAL>FLOAT                  { myToken(KWFLOAT); }
<INITIAL>BOOL                   { myToken(KWBOOL); }
<INITIAL>STRING                 { myToken(KWSTRING); }
<INITIAL>IDENT                  { myToken(KWIDENT); }
<INITIAL>NEWLINE                { myToken(KWNEWLINE); }
<INITIAL>"::"                   { myToken(KWDOUBLE_COLON); }
<INITIAL>"->"                   { myToken(KWARROW); }

<INITIAL>\"([^"]|\\.)*\"        { myDebug("KEYWORD %s\n", xptext);
                                  xplval.symVal = utSymCreate(unmunge(xptext));
                                  return KEYWORD; }
<INITIAL>[0-9]+                 { xplval.intVal = atol(xptext);
                                  myDebug("INTEGER %u\n", xplval.intVal);
                                  return INTEGER; }
<INITIAL>[a-zA-Z_][a-zA-Z0-9_]* { myDebug("NONTERM %s\n", xptext);
                                  xplval.symVal = utSymCreate(xptext);
                                  return NONTERM; }
<INITIAL>.                      { myDebug("Char '%c'\n", xptext[0]);
                                  return xptext[0]; }
