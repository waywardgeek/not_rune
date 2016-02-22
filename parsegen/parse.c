#include "parse_int.h"

uint32 xpLineNum;
FILE *xpFile;
xyParser xpCurrentParser;
utSym xpINTSym, xpUINTSym, xpFLOATSym, xpDOUBLESym, xpBOOLSym, xpSTRINGSym, xpLISTSym, xpIDENTSym;
utSym xpEOFSym, xpEMPTYSym;

static void initSyms(void) {
    xpINTSym = utSymCreate("INT");
    xpUINTSym = utSymCreate("UINT");
    xpFLOATSym = utSymCreate("FLOAT");
    xpDOUBLESym = utSymCreate("DOUBLE");
    xpBOOLSym = utSymCreate("BOOL");
    xpSTRINGSym = utSymCreate("STRING");
    xpLISTSym = utSymCreate("LIST");
    xpIDENTSym = utSymCreate("IDENT");
    xpEOFSym = utSymCreate("EOF");
    xpEMPTYSym = utSymCreate("EMPTY");
}

// Print a token.
void xpPrintToken(xpToken token) {
    xyMtoken mtoken = xpTokenGetMtoken(token);
    if(xyMtokenGetType(mtoken) == XY_KEYWORD) {
        printf("\"%s\"", xyMtokenGetName(mtoken));
    } else {
        printf("%s", xyMtokenGetName(mtoken));
    }
}

// Print a rule.
void xpPrintRule(xpRule rule) {
    printf("%s:", xpRuleGetName(rule));
    xpProduction production;
    bool firstTime = true;
    xpForeachRuleProduction(rule, production) {
        if(!firstTime) {
            printf("|");
        }
        firstTime = false;
        xpToken token;
        xpForeachProductionToken(production, token) {
            putchar(' ');
            xpPrintToken(token);
        } xpEndProductionToken;
        putchar('\n');
    } xpEndRuleProduction;
    printf(";\n");
}

// Print all rules.
void xpPrintParser(xyParser parser) {
    xpRule rule;
    xpForeachParserRule(parser, rule) {
        xpPrintRule(rule);
        putchar('\n');
    } xpEndParserRule;
    xpItemset itemset;
    xpForeachParserItemset(parser, itemset) {
        xpPrintItemset(itemset);
        printf("\n");
    } xpEndParserItemset;
    xyMtoken mtoken;
    xyForeachParserMtoken(parser, mtoken) {
        if(xyMtokenGetType(mtoken) == XY_NONTERM) {
            printf("FIRST[%s] = ", xyMtokenGetName(mtoken));
            xpTset tset = xpMtokenGetFirstTset(mtoken);
            xpTentry tentry;
            xpForeachTsetTentry(tset, tentry) {
                printf(" %s", xyMtokenGetName(xpTentryGetMtoken(tentry)));
            } xpEndTsetTentry;
            if(xpTsetHasEmpty(tset)) {
                printf(" EMPTY");
            }
            putchar('\n');
        }
    } xyEndParserMtoken;
}

// Create a token object.  Find an Mtoken it matches, and create it if not found.
xpToken xpTokenCreate(xpProduction production, xyMtokenType type, utSym sym) {
    xyParser parser = xpRuleGetParser(xpProductionGetRule(production));
    xyMtoken mtoken = xyParserFindMtoken(parser, type, sym);
    if(mtoken == xyMtokenNull) {
        mtoken = xyMtokenCreate(parser, type, sym);
    }
    xpToken token = xpTokenAlloc();
    xpMtokenAppendToken(mtoken, token);
    xpProductionAppendToken(production, token);
    return token;
}

bool xpParseGrammar(char *fileName) {
    xpDatabaseStart();
    initSyms();
    xpLineNum = 1;
    xpFile = fopen(fileName, "r");
    xpCurrentParser = xyParserCreate(utSymCreate("rules"));
    if(xpFile == NULL) {
        fprintf(stderr, "Unable to open file %s for reading\n", fileName);
        return false;
    }
    int rc = xpparse();
    fclose(xpFile);
    if(rc != 0) {
        xyParserDestroy(xpCurrentParser);
        xyDatabaseStop();
        return false;
    }
    xpBuildParserActionGotoTable(xpCurrentParser);
    xyPrintParser(xpCurrentParser);
    xpDatabaseStop();
    return true;
}

bool xpParse(char *fileName) {
    return true;
}
