#include "rune.h"

uint32 xyLineNum;
FILE *xyFile;
xyParser xyCurrentParser;

// Get the name of an xyValueType.
char *xyValueTypeGetName(xyValueType type) {
    switch(type) {
    case XY_INT: return "INT";
    case XY_UINT: return "UINT";
    case XY_FLOAT: return "FLOAT";
    case XY_DOUBLE: return "DOUBLE";
    case XY_BOOL: return "BOOL";
    case XY_STRING: return "STRING";
    case XY_LIST: return "LIST";
    case XY_SYM: return "SYM";
    default:
        utExit("Unknown value type");
    }
    return NULL; // Dummy return
}

// Print a token.
void xyPrintToken(xyToken token) {
    xyMtoken mtoken = xyTokenGetMtoken(token);
    utSym sym = xyMtokenGetSym(mtoken);
    if(xyMtokenGetType(mtoken) == XY_KEYWORD) {
        printf("\"%s\"", utSymGetName(sym));
    } else {
        printf("%s", utSymGetName(sym));
    }
}

// Print a rule.
void xyPrintRule(xyRule rule) {
    printf("%s:", xyRuleGetName(rule));
    xyProduction production;
    bool firstTime = true;
    xyForeachRuleProduction(rule, production) {
        if(!firstTime) {
            printf("|");
        }
        firstTime = false;
        xyToken token;
        xyForeachProductionToken(production, token) {
            putchar(' ');
            xyPrintToken(token);
        } xyEndProductionToken;
        putchar('\n');
    } xyEndRuleProduction;
    printf(";\n");
}

// Print all rules.
void xyPrintParser(xyParser parser) {
    xyRule rule;
    xyForeachParserRule(parser, rule) {
        xyPrintRule(rule);
        putchar('\n');
    } xyEndParserRule;
    xyItemset itemset;
    xyForeachParserItemset(parser, itemset) {
        xyPrintItemset(itemset);
        printf("\n");
    } xyEndParserItemset;

}

// Create a new master token object on the parser.
xyMtoken xyMtokenCreate(xyParser parser, xyMtokenType type, utSym sym) {
    xyMtoken mtoken = xyMtokenAlloc();
    xyMtokenSetType(mtoken, type);
    xyMtokenSetSym(mtoken, sym);
    xyParserAppendMtoken(parser, mtoken);
    return mtoken;
}

// Create a token object.  Find an Mtoken it matches, and create it if not found.
xyToken xyTokenCreate(xyProduction production, xyMtokenType type, utSym sym) {
    xyParser parser = xyRuleGetParser(xyProductionGetRule(production));
    xyMtoken mtoken = xyParserFindMtoken(parser, type, sym);
    if(mtoken == xyMtokenNull) {
        mtoken = xyMtokenCreate(parser, type, sym);
    }
    xyToken token = xyTokenAlloc();
    xyMtokenAppendToken(mtoken, token);
    xyProductionAppendToken(production, token);
    return token;
}

bool xyParseGrammar(char *fileName) {
    xyLineNum = 1;
    xyFile = fopen(fileName, "r");
    xyCurrentParser = xyParserCreate(utSymCreate("rules"));
    if(xyFile == NULL) {
        fprintf(stderr, "Unable to open file %s for reading\n", fileName);
        return false;
    }
    int rc = xyparse();
    fclose(xyFile);
    if(rc != 0) {
        xyParserDestroy(xyCurrentParser);
        return false;
    }
    xyBuildItemSets(xyCurrentParser);
    return true;
}

bool xyParse(char *fileName) {
    return true;
}
