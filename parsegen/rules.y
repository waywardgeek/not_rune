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

%type <mapVal> map concatExpr concatExprs appendExpr tokenExpr listExpr

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
    xyMtoken mtoken = xyMtokenCreate(xpCurrentParser, XY_NONTERM, $1);
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

concatExpr: appendExpr
| concatExpr '+' appendExpr
{
    $$ = xyMapCreate(xpCurrentParser, XY_MAP_CONCAT);
    xyMapAppendMap($$, $1);
    xyMapAppendMap($$, $3);
}
;

appendExpr: tokenExpr
| appendExpr '.' tokenExpr
{
    $$ = xyMapCreate(xpCurrentParser, XY_MAP_APPEND);
    xyMapAppendMap($$, $1);
    xyMapAppendMap($$, $3);
}
;

tokenExpr: '$' INTEGER
{
    if($2 < 1 || $2 > xpProductionGetUsedToken(xpCurrentProduction)) {
        xperror("$%u is out of range", $2);
    }
    $$ = xyMapCreate(xpCurrentParser, XY_MAP_TOKEN);
    xyMapSetPosition($$, $2 - 1);
}
| listExpr
| KEYWORD
{
    $$ = xyMapCreate(xpCurrentParser, XY_MAP_KEYWORD);
    xyMapSetSym($$, $1);
}
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
    xpTokenCreate(xpCurrentProduction, XY_INT, utSymNull);
}
| KWFLOAT
{
    xpTokenCreate(xpCurrentProduction, XY_FLOAT, utSymNull);
}
| KWBOOL
{
    xpTokenCreate(xpCurrentProduction, XY_BOOL, utSymNull);
}
| KWSTRING
{
    xpTokenCreate(xpCurrentProduction, XY_STRING, utSymNull);
}
| KWIDENT
{
    xpTokenCreate(xpCurrentProduction, XY_IDENT, utSymNull);
}
| KWNEWLINE
{
    xpTokenCreate(xpCurrentProduction, XY_NEWLINE, utSymNull);
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
