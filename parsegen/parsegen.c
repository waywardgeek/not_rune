#include "parsegen_int.h"

uint32 xpLineNum;
FILE *xpFile;
xyParser xpCurrentParser;

// Print a token.
void xpPrintToken(xpToken token) {
    xyMtoken mtoken = xpTokenGetMtoken(token);
    if(xyMtokenGetType(mtoken) == XY_TOK_KEYWORD) {
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
        xyMap map = xpProductionGetMap(production);
        if(map != xyMapNull) {
            printf(" -> ");
            xyPrintMap(map);
        }
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
        putchar('\n');
    } xpEndParserItemset;
    xyMtoken mtoken;
    xyForeachParserMtoken(parser, mtoken) {
        if(xyMtokenGetType(mtoken) == XY_TOK_NONTERM) {
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
    putchar('\n');
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

// Report an error if there are any unused rules.
static void checkForUnusedRules(xyParser parser) {
    bool firstTime = true;
    xpRule rule;
    xpForeachParserRule(parser, rule) {
        if(!firstTime) {
            xyMtoken mtoken = xpRuleGetMtoken(rule);
            if(xpMtokenGetFirstToken(mtoken) == xpTokenNull) {
                utError("Unused rule %s", xyMtokenGetName(mtoken));
            }
        }
        firstTime = false;
    } xpEndParserRule;
}

xyParser xpParseGrammar(char *fileName) {
    xpDatabaseStart();
    xpLineNum = 1;
    xpFile = fopen(fileName, "r");
    xpCurrentParser = xyParserCreate(utSymCreate("rules"));
    if(xpFile == NULL) {
        fprintf(stderr, "Unable to open file %s for reading\n", fileName);
        return xyParserNull;
    }
    int rc = xpparse();
    fclose(xpFile);
    if(rc != 0) {
        xyParserDestroy(xpCurrentParser);
        xyDatabaseStop();
        return xyParserNull;
    }
    checkForUnusedRules(xpCurrentParser);
    if(!xpBuildParserActionGotoTable(xpCurrentParser)) {
        utExit("Exiting due to syntax conflicts");
    }
    xpPrintParser(xpCurrentParser);
    xyPrintParser(xpCurrentParser);
    xpDatabaseStop();
    return xpCurrentParser;
}
