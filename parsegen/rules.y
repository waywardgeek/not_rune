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
    xyMap mapVal;
};

%token <symVal> NONTERM KEYWORD
%token <intVal> INTEGER

%type <mapVal> map concatExpr concatExprs valueExpr listExpr

%token KWINTEGER KWFLOAT KWBOOL KWSTRING KWIDENT KWNEWLINE KWDOUBLE_COLON KWARROW

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

productionBody: tokens optMap
;

optMap: // Empty
| KWARROW map
{
    xpProductionSetMap(xpCurrentProduction, $2);
}
;

map: concatExpr
;

concatExprs: // Empty
{
    $$ = xyMapCreate(xpCurrentParser, XY_MAP_LIST);
}
| concatExprs concatExpr
{
    $$ = $1;
    xyMapAppendMap($1, $2);
}
;

concatExpr: valueExpr
| concatExpr '.' valueExpr
{
    $$ = xyMapCreate(xpCurrentParser, XY_MAP_CONCAT);
    xyMapAppendMap($$, $1);
    xyMapAppendMap($$, $3);
}
;

valueExpr: '$' INTEGER
{
    if($2 < 1 || $2 > xpProductionGetUsedToken(xpCurrentProduction)) {
        xperror("$%u is out of range", $2);
    }
    $$ = xyMapCreate(xpCurrentParser, XY_MAP_VALUE);
    xyMapSetPosition($$, $2 - 1);
}
| listExpr
;

listExpr: '(' concatExprs ')'
{
    $$ = $2;
}
;

tokens: // Empty
| tokens token
;

token: KWINTEGER
{
    xpTokenCreate(xpCurrentProduction, XY_TOK_INT, utSymNull);
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
