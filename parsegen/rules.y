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

%token INT FLOAT BOOL STRING IDENT NEWLINE IGNORE_NEWLINES DOUBLE_COLON

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

firstProduction: tokens optModifiers
;

production: productionHeader tokens optModifiers
;

optModifiers: /* Empty */
| DOUBLE_COLON modifiers
;

modifiers: /* Empty */
| modifiers modifier
;

modifier: IGNORE_NEWLINES
{
    xpProductionSetIgnoreNewlines(xpCurrentProduction, true);
}
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
    xpTokenCreate(xpCurrentProduction, XY_TOK_INTEGER, utSymNull);
}
| FLOAT
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_FLOAT, utSymNull);
}
| BOOL
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_BOOL, utSymNull);
}
| STRING
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_STRING, utSymNull);
}
| IDENT
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_IDENT, utSymNull);
}
| NEWLINE
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_NEWLINE, utSymNull);
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
