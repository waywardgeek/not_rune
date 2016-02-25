This directory holds the "core" Rune compiler.  It only recognizes the "core"
list form of Rune code, and only low-level forms that can be directly
translated to C code.

Syntax
------
file: {importStatement} {statement}
importStatement: ("import" PATH)
statement: executableStatement | declaration
executableStatement: assignStatement | switchStatement | callStatement | returnStatement
    | deleteStatement
assignStatement: ("=" accessExpr expr)
ifStatement: ("if" boolExpr statementList [statementList])
whileStatement: (["do" statementList] "while" boolExpr [statementList])
callStatement: functionCall
returnStatement: ("return" expr)
deleteStatement: ("delete" expr)
switchStatement: ("switch" arithExpr {(constArithExpr statementList)} [("default" statementList)])
declaration: varDecl | functionDecl | classDecl | structDecl
varDecl: ("var" type IDENT [expr])
functionDecl: ("func" type IDENT({functionParamDecl}) statementList)
functionParamDecl: type IDENT
classDecl: ("class" {declaration})
structDecl: ("struct" IDENT {type IDENT})
statementList: ({statement})
functionCall: (PATH {expr})
type: PATH
expr: boolExpr | arithExpr
accessExpr: PATH | ("index" accessExpr arithExpr) | ("dot" accessExpr IDENT)
boolExpr: logicalExpr || relationExpr
logicalExpr: ("||" {boolExpr}) | ("&&" {boolExpr}) | ("!" boolExpr) | accessExpr
relationalExpr: ("==" boolExpr boolExpr) | ("!=" boolExpr boolExpr)
    | (relationalOp arithExpr arithExpr)
relationalOp: "==" | "!=" | ">" | "<" | ">=" | "<="
arithExpr: (binaryArithOp arithExpr arithExpr) | (unaryArithOp arithExpr) | accessExpr
biaryArithOp: "+" | "-" | "*" | "&" | "|" | "^" | "/" | "%" | "<<" | ">>"
unaryArithOp: "~" | "-"
constArithExpr: PATH | INTEGER

The lexer needs to recognize PATH as a series of "." separated IDENTs.  The
built-in types are just global identifiers, as is "true" and "false".  They are:

int - big integer
int<n> - n-bit signed int, where <n> is a number from 1 to 65537.
uint<n> - n-bit unsigned int, where <n> is a number from 1 to 65537.
float - C float
double - C double
bool - boolean
string - UTF8 encoded string
list - dynamic array of value
value - a union type capable of holding any built-in types other than value
sym - a symbol in the global symbol table

TODO: enhance lexer to support integer types, integer constants, and PATH tokens.
