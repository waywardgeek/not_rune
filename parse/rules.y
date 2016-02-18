%{

#include "parse_int.h"

static xpRule xpCurrentRule;
static xpProduction xpCurrentProduction;

/*--------------------------------------------------------------------------------------------------
  Provide yyerror function capability.
--------------------------------------------------------------------------------------------------*/
void xperror(
    char *message,
    ...)
{
    char *buff;
    va_list ap;

    va_start(ap, message);
    buff = utVsprintf(message, ap);
    va_end(ap);
    utError("Line %d, token \"%s\": %s", xpLineNum, xptext, buff);
}

%}

%union {
    utSym symVal;
};

%token <symVal> NONTERM KEYWORD

%token INT UINT FLOAT DOUBLE BOOL STRING IDENT

%%

goal: rules
;

rules: // Empty
| rules rule
;

rule: ruleHeader productions ';'
;

ruleHeader: NONTERM ':'
{
    xpCurrentRule = xpRuleAlloc();
    xpRuleSetSym(xpCurrentRule, $1);
    xpParserAppendRule(xpCurrentParser, xpCurrentRule);
    xpCurrentProduction = xpProductionAlloc();
    xpRuleAppendProduction(xpCurrentRule, xpCurrentProduction);
}
;

productions: firstProduction
| productions production
;

firstProduction: tokens
;

production: productionHeader tokens
;

productionHeader: '|'
{
    xpCurrentProduction = xpProductionAlloc();
    xpRuleAppendProduction(xpCurrentRule, xpCurrentProduction);
}

tokens: // Empty
| tokens token
;

token: INT
{
    xpTokenCreate(xpCurrentProduction, XY_TERM, xpINTSym);
}
| UINT
{
    xpTokenCreate(xpCurrentProduction, XY_TERM, xpUINTSym);
}
| FLOAT
{
    xpTokenCreate(xpCurrentProduction, XY_TERM, xpFLOATSym);
}
| DOUBLE
{
    xpTokenCreate(xpCurrentProduction, XY_TERM, xpDOUBLESym);
}
| BOOL
{
    xpTokenCreate(xpCurrentProduction, XY_TERM, xpBOOLSym);
}
| STRING
{
    xpTokenCreate(xpCurrentProduction, XY_TERM, xpSTRINGSym);
}
| IDENT
{
    xpTokenCreate(xpCurrentProduction, XY_TERM, xpIDENTSym);
}
| NONTERM
{
    xpTokenCreate(xpCurrentProduction, XY_NONTERM, $1);
}
| KEYWORD
{
    xpTokenCreate(xpCurrentProduction, XY_KEYWORD, $1);
}
;

%%
