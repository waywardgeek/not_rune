#include "database.h"
#include "parsegen.h"
#include "xpdatabase.h"

// Rule parsing
void xpPrintToken(xpToken token);
void xpPrintRule(xpRule rule);
void xpPrintParser(xyParser parser);
xpToken xpTokenCreate(xpProduction production, xyMtokenType type, utSym sym);
void xpBuildParserActionGotoTable(xyParser parser);

// Item sets
void xpBuildItemsets(xyParser parser);
void xpPrintItem(xpItem item);
void xpPrintItemset(xpItemset itemset);

// Shortcuts
static inline char *xpRuleGetName(xpRule rule) {return xyMtokenGetName(xpRuleGetMtoken(rule));}

// Lex, Yacc stuff
extern uint32 xpLineNum;
extern int xpparse(void);
extern int xplex(void); /* A wrapper around dvlexlex */
extern void xperror(char *message, ...);
extern char *xptext;
extern FILE *xpFile;
extern xyParser xpCurrentParser;
