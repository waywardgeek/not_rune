%{

#include "parsegen_int.h"

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

%token INT FLOAT BOOL STRING IDENT

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
    xyMtoken mtoken = xyMtokenCreate(xpCurrentParser, XY_TOK_NONTERM, $1);
    xpRuleInsertMtoken(xpCurrentRule, mtoken);
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
    xpTermTokenCreate(xpCurrentProduction, XY_TOK_INT);
}
| FLOAT
{
    xpTermTokenCreate(xpCurrentProduction, XY_TOK_FLOAT);
}
| BOOL
{
    xpTermTokenCreate(xpCurrentProduction, XY_TOK_BOOL);
}
| STRING
{
    xpTermTokenCreate(xpCurrentProduction, XY_TOK_STRING);
}
| IDENT
{
    xpTermTokenCreate(xpCurrentProduction, XY_TOK_IDENT);
}
| NONTERM
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_NONTERM, $1);
}
| KEYWORD
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_KEYWORD, $1);
}
;

%%
