%{

#include "rune.h"

xyRule xyCurrentRule;
xyProduction xyCurrentProduction;

/*--------------------------------------------------------------------------------------------------
  Provide yyerror function capability.
--------------------------------------------------------------------------------------------------*/
void xyerror(
    char *message,
    ...)
{
    char *buff;
    va_list ap;

    va_start(ap, message);
    buff = utVsprintf(message, ap);
    va_end(ap);
    utError("Line %d, token \"%s\": %s", xyLineNum, xytext, buff);
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
    xyCurrentRule = xyRuleAlloc();
    xyRuleSetSym(xyCurrentRule, $1);
    xyParserAppendRule(xyCurrentParser, xyCurrentRule);
    xyCurrentProduction = xyProductionAlloc();
    xyRuleAppendProduction(xyCurrentRule, xyCurrentProduction);
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
    xyCurrentProduction = xyProductionAlloc();
    xyRuleAppendProduction(xyCurrentRule, xyCurrentProduction);
}

tokens: // Empty
| tokens token
;

token: INT
{
    xyTokenCreate(xyCurrentProduction, XY_TERM, xyINTSym);
}
| UINT
{
    xyTokenCreate(xyCurrentProduction, XY_TERM, xyUINTSym);
}
| FLOAT
{
    xyTokenCreate(xyCurrentProduction, XY_TERM, xyFLOATSym);
}
| DOUBLE
{
    xyTokenCreate(xyCurrentProduction, XY_TERM, xyDOUBLESym);
}
| BOOL
{
    xyTokenCreate(xyCurrentProduction, XY_TERM, xyBOOLSym);
}
| STRING
{
    xyTokenCreate(xyCurrentProduction, XY_TERM, xySTRINGSym);
}
| IDENT
{
    xyTokenCreate(xyCurrentProduction, XY_TERM, xyIDENTSym);
}
| NONTERM
{
    xyTokenCreate(xyCurrentProduction, XY_NONTERM, $1);
}
| KEYWORD
{
    xyTokenCreate(xyCurrentProduction, XY_KEYWORD, $1);
}
;

%%
