#include "rune.h"

// Print the item.
void xyPrintItem(xyItem item) {
    xyProduction production = xyItemGetProduction(item);
    xyRule rule = xyProductionGetRule(production);
    if(!xyItemCore(item)) {
        printf("+ ");
    }
    printf("%s ->", xyRuleGetName(rule));
    xyToken token;
    uint32 position = 0;
    uint32 dotPosition = xyItemGetDotPosition(item);
    xyForeachProductionToken(production, token) {
        if(position == dotPosition) {
            printf(" .");
        }
        putchar(' ');
        xyPrintToken(token);
        position++;
    } xyEndProductionToken;
    if(position == dotPosition) {
        printf(" .");
    }
    putchar('\n');
}

// Print the itemset.
void xyPrintItemset(xyItemset itemset) {
    printf("Itemset %u\n", xyItemset2Index(itemset));
    xyItem item;
    xyForeachItemsetItem(itemset, item) {
        printf("    ");
        xyPrintItem(item);
    } xyEndItemsetItem;
}

// Create a new parser.
xyParser xyParserCreate(utSym sym) {
    xyParser parser = xyParserAlloc();
    xyParserSetSym(parser, sym);
    xyRootAppendParser(xyTheRoot, parser);
    return parser;
}

// Create a new itemset.
xyItemset xyItemSetCreate(xyParser parser) {
    xyItemset itemset = xyItemsetAlloc();
    xyParserAppendItemset(parser, itemset);
    return itemset;
}

// Add the production to the itemset.
xyItem xyItemCreate(xyItemset itemset, xyProduction production, uint32 position, bool core) {
    xyItem item = xyItemAlloc();
    xyItemSetDotPosition(item, position);
    xyItemSetCore(item, core);
    xyProductionAppendItem(production, item);
    xyItemsetAppendItem(itemset, item);
    return item;
}

// Create a transition object between itemsets.
xyTransition xyTransitionCreate(xyMtoken mtoken, xyItemset fromItemset, xyItemset toItemset) {
    xyTransition transition = xyTransitionAlloc();
    xyMtokenAppendTransition(mtoken, transition);
    xyItemsetAppendOutTransition(fromItemset, transition);
    xyItemsetAppendInTransition(toItemset, transition);
    return transition;
}

// Determine if the item is already in the itemset.
static bool itemInItemset(xyItemset itemset, xyProduction production, uint32 dotPosition) {
    xyItem item;
    xyForeachItemsetItem(itemset, item) {
        if(xyItemGetProduction(item) == production && xyItemGetDotPosition(item) == dotPosition) {
            return true;
        }
    } xyEndItemsetItem;
    return false;
}

// Add the rule's productions to the itemset.
static void addRuleToItemset(xyItemset itemset, xyRule rule, bool core) {
    xyProduction production;
    xyForeachRuleProduction(rule, production) {
        xyItemCreate(itemset, production, 0, core);
    } xyEndRuleProduction;
}

// Add additional items to the itemset until there are no productions that have
// not been used to expand the itemset.  A production can only be included
// once.
static void computeClosure(xyParser parser, xyItemset itemset) {
    xyItem item;
    xyForeachItemsetItem(itemset, item) {
        xyProduction production = xyItemGetProduction(item);
        uint32 position = xyItemGetDotPosition(item);
        if(position < xyProductionGetUsedToken(production)) {
            xyToken token = xyProductionGetiToken(production, position);
            xyMtoken mtoken = xyTokenGetMtoken(token);
            if(xyMtokenGetType(mtoken) == XY_NONTERM) {
                utSym ruleSym = xyMtokenGetSym(mtoken);
                xyRule rule = xyParserFindRule(parser, ruleSym);
                if(rule == xyRuleNull) {
                    utError("Undefined non-terminal %s", utSymGetName(ruleSym));
                }
                addRuleToItemset(itemset, rule, false);
            }
        }
    } xyEndItemsetItem;
}

// Build new itemsets that we can transition to from this one.
static void buildNewItemSetsFromItemset(xyParser parser, xyItemset itemset) {
    //printf("Building transitions from itemset:");
    //xyPrintItemset(itemset);
    xyItem item;
    xyForeachItemsetItem(itemset, item) {
        uint32 position = xyItemGetDotPosition(item);
        xyProduction production = xyItemGetProduction(item);
        if(position < xyProductionGetUsedToken(production)) {
            xyToken token = xyProductionGetiToken(production, position);
            xyMtoken mtoken = xyTokenGetMtoken(token);
            xyTransition transition = xyItemsetFindOutTransition(itemset, mtoken);
            xyItemset destItemset;
            if(transition == xyTransitionNull) {
                //printf("New itemset for ");
                //xyPrintItem(item);
                destItemset = xyItemSetCreate(parser);
                transition = xyTransitionCreate(mtoken, itemset, destItemset);
            } else {
                //printf("Found existing itemset for ");
                //xyPrintItem(item);
                destItemset = xyTransitionGetToItemset(transition);
            }
            if(!itemInItemset(destItemset, production, position + 1)) {
                xyItemCreate(destItemset, production, position + 1, true);
            }
        }
    } xyEndItemsetItem;
}

// Determine if the itemsets have the same core rules, with the same dot positions.
static bool itemsetsHaveSameCore(xyItemset itemset1, xyItemset itemset2) {
    xyItem item;
    uint32 numCores1 = 0;
    for(item = xyItemsetGetFirstItem(itemset1); item != xyItemNull && xyItemCore(item);
            item = xyItemGetNextItemsetItem(item)) {
        xyProduction production = xyItemGetProduction(item);
        uint32 dotPosition = xyItemGetDotPosition(item);
        if(!itemInItemset(itemset2, production, dotPosition)) {
            return false;
        }
        numCores1++;
    }
    uint32 numCores2 = 0;
    for(item = xyItemsetGetFirstItem(itemset2); item != xyItemNull && xyItemCore(item);
            item = xyItemGetNextItemsetItem(item)) {
        numCores2++;
    }
    return numCores1 == numCores2;
}

// Find an existing itemset with the same core productions.
static xyItemset findExistingIdenticalItemset(xyItemset newItemset) {
    xyProduction firstProduction = xyItemGetProduction(xyItemsetGetFirstItem(newItemset));
    xyItem item;
    xyForeachProductionItem(firstProduction, item) {
        xyItemset oldItemset = xyItemGetItemset(item);
        if(oldItemset != newItemset && itemsetsHaveSameCore(newItemset, oldItemset)) {
            return oldItemset;
        }
    } xyEndProductionItem;
    return xyItemsetNull;
}

// If outgoing transitions lead to itemsets that have identical cores to
// existing itemsets, merge them.
static void mergeItemsetsWithIdenticalCores(xyItemset itemset) {
    xyTransition transition;
    xyItemset destItemset;
    xyForeachItemsetOutTransition(itemset, transition) {
        destItemset = xyTransitionGetToItemset(transition);
        xyItemset oldItemset = findExistingIdenticalItemset(destItemset);
        if(oldItemset != xyItemsetNull) {
            xyItemsetRemoveInTransition(destItemset, transition);
            xyItemsetAppendInTransition(oldItemset, transition);
            xyItemsetDestroy(destItemset);
        }
    } xyEndItemsetOutTransition;
}

// Compute all the itemsets, starting with just the goal itemset, by computing
// closures and then new sets.  LR(0) sets are the same if they are generated
// from the same items.
static void computeLR0Sets(xyParser parser) {
    xyItemset itemset;
    xyForeachParserItemset(parser, itemset) {
        // Note that we append itemsets to the parser in this loop.
        computeClosure(parser, itemset);
        buildNewItemSetsFromItemset(parser, itemset);
        mergeItemsetsWithIdenticalCores(itemset);
    } xyEndParserItemset;
}

void xyBuildItemSets(xyParser parser) {
    xyRule goal = xyParserGetFirstRule(parser);
    xyItemset goalSet = xyItemSetCreate(parser);
    addRuleToItemset(goalSet, goal, true);
    computeLR0Sets(parser);
    xyPrintParser(parser);
}
