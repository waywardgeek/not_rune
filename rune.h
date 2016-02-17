#include "xydatabase.h"

bool xyParse(char *fileName);

// Strings
xyString xyStringCreate(uint8 *text, uint32 len);

// Rule parsing
bool xyParseGrammar(char *fileName);
xyToken xyTokenCreate(xyProduction production, xyMtokenType type, utSym sym);
char *xyValueTypeGetName(xyValueType type);
void xyPrintToken(xyToken token);
void xyPrintRule(xyRule rule);
void xyPrintParser(xyParser parser);

// Item sets
xyParser xyParserCreate(utSym sym);
void xyBuildItemsets(xyParser parser);
void xyPrintItem(xyItem item);
void xyPrintItemset(xyItemset itemset);

// Shortcuts
static inline char *xyMtokenGetName(xyMtoken mtoken) {return utSymGetName(xyMtokenGetSym(mtoken));}

// Globals
extern xyRoot xyTheRoot;
extern utSym xyINTSym, xyUINTSym, xyFLOATSym, xyDOUBLESym, xyBOOLSym, xySTRINGSym, xyLISTSym, xyIDENTSym;
extern utSym xyEOFSym, xyEMPTYSym;

// Lex, Yacc stuff
extern uint32 xyLineNum;
extern int xyparse(void);
extern int xylex(void); /* A wrapper around dvlexlex */
extern void xyerror(char *message, ...);
extern char *xytext;
extern FILE *xyFile;
extern xyParser xyCurrentParser;
