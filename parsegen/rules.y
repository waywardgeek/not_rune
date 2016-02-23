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
    uint64 intVal;
};

%token <symVal> NONTERM KEYWORD
%token <intVal> INTEGER

%token KWINTEGER KWFLOAT KWBOOL KWSTRING KWIDENT KWNEWLINE KWIGNORE_NEWLINES KWDOUBLE_COLON KWARROW

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

firstProduction: productionBody
;

production: productionHeader productionBody
;

productionHeader: '|'
{
    xpCurrentProduction = xpProductionAlloc();
    xpRuleAppendProduction(xpCurrentRule, xpCurrentProduction);
}

productionBody: tokens optModifiers optMapping
;

optModifiers: // Empty
| KWDOUBLE_COLON modifiers
;

modifiers: /* Empty */
| modifiers modifier
;

modifier: KWIGNORE_NEWLINES
{
    xpProductionSetIgnoreNewlines(xpCurrentProduction, true);
}
;

optMapping: // Empty
| KWARROW mapping
;

mapping: concatExpr
;

concatExprs: // Empty
| concatExprs concatExpr
;

concatExpr: valueExpr
| concatExpr '.' valueExpr
;

valueExpr: '$' INTEGER
| list
;

list: '(' concatExprs ')'
;

tokens: // Empty
| tokens token
;

token: KWINTEGER
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_INTEGER, utSymNull);
}
| KWFLOAT
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_FLOAT, utSymNull);
}
| KWBOOL
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_BOOL, utSymNull);
}
| KWSTRING
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_STRING, utSymNull);
}
| KWIDENT
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_IDENT, utSymNull);
}
| KWNEWLINE
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
