/*
  The algorithm here is mostly from these two sites:

  https://lambda.uta.edu/cse5317/notes/node20.html
  http://web.cs.dal.ca/~sjackson/lalr1.html

  The lambda.uta.edu article mentioned simply building the LR(0) set, and then
  adding the "lookahead" sets as a post-process, which is done here.  More
  details of how to make this all work is found on the other site.
*/
#include "rune.h"

// Print the item.
void xyPrintItem(xyItem item) {
    printf("%u: ", xyItem2Index(item));
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
    xyTset lookaheads = xyItemGetLookaheadTset(item);
    if(lookaheads != xyTsetNull) {
        printf(" [");
        xyTitem titem;
        bool firstTime = true;
        xyForeachTsetTitem(lookaheads, titem) {
            if(!firstTime) {
                putchar(' ');
            }
            firstTime = false;
            printf("%s", xyMtokenGetName(xyTitemGetMtoken(titem)));
        } xyEndTsetTitem;
        printf("]");
    }
    bool firstTime = true;
    xyIedge iedge;
    xyForeachItemOutIedge(item, iedge) {
        if(firstTime) {
            printf(" :");
            firstTime = false;
        }
        printf(" %u", xyItem2Index(xyIedgeGetToItem(iedge)));
    } xyEndItemOutIedge;
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
xyItemset xyItemsetCreate(xyParser parser) {
    xyItemset itemset = xyItemsetAlloc();
    xyParserAppendItemset(parser, itemset);
    return itemset;
}

// Add the production to the itemset.
xyItem xyItemCreate(xyItemset itemset, xyItem prevItem, xyProduction production, uint32 position, bool core) {
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

// Create a new dependency edge between two items.
xyIedge xyIedgeCreate(xyItem fromItem, xyItem toItem) {
    xyIedge iedge = xyIedgeAlloc();
    xyItemAppendOutIedge(fromItem, iedge);
    xyItemAppendInIedge(toItem, iedge);
    return iedge;
}

// Create a new item in a token set.
xyTitem xyTitemCreate(xyTset tset, xyMtoken mtoken) {
    xyTitem titem = xyTsetFindTitem(tset, mtoken);
    if(titem != xyTitemNull) {
        // Already added it
        return titem;
    }
    titem = xyTitemAlloc();
    xyTitemSetMtoken(titem, mtoken);
    xyTsetInsertTitem(tset, titem);
    return titem;
}

// Add the EOF token to each production in the rule.  This is done just for the
// first goal rule.
static void addEOFTokenToLookaheads(xyRule rule) {
    xyParser parser = xyRuleGetParser(rule);
    xyMtoken eofMtoken = xyMtokenCreate(parser, XY_TERM, xyEOFSym);
    xyProduction production;
    xyForeachRuleProduction(rule, production) {
        xyItem item;
        xyForeachProductionItem(production, item) {
            xyTset tset = xyTsetAlloc();
            xyItemSetLookaheadTset(item, tset);
            xyTitemCreate(tset, eofMtoken);
            xyParserAppendUpdatedItem(parser, item);
        } xyEndProductionItem;
    } xyEndRuleProduction;
}

// Determine if the item is already in the itemset.
static xyItem findItemInItemset(xyItemset itemset, xyProduction production, uint32 dotPosition) {
    xyItem item;
    xyForeachItemsetItem(itemset, item) {
        if(xyItemGetProduction(item) == production && xyItemGetDotPosition(item) == dotPosition) {
            return item;
        }
    } xyEndItemsetItem;
    return xyItemNull;
}

// Add the rule's productions to the itemset.
static void addRuleToItemset(xyItemset itemset, xyItem prevItem, xyRule rule, bool core) {
    xyProduction production;
    xyForeachRuleProduction(rule, production) {
        xyItem item = findItemInItemset(itemset, production, 0);
        if(item == xyItemNull) {
            item = xyItemCreate(itemset, prevItem, production, 0, core);
        }
        if(prevItem != xyItemNull) {
            xyIedgeCreate(prevItem, item);
        }
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
                addRuleToItemset(itemset, item, rule, false);
            }
        }
    } xyEndItemsetItem;
}

// Build new itemsets that we can transition to from this one.
static void buildNewItemsetsFromItemset(xyParser parser, xyItemset itemset) {
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
                destItemset = xyItemsetCreate(parser);
                transition = xyTransitionCreate(mtoken, itemset, destItemset);
            } else {
                //printf("Found existing itemset for ");
                //xyPrintItem(item);
                destItemset = xyTransitionGetToItemset(transition);
            }
            xyItem destItem = findItemInItemset(destItemset, production, position + 1);
            if(destItem == xyItemNull) {
                destItem = xyItemCreate(destItemset, item, production, position + 1, true);
            }
            xyIedgeCreate(item, destItem);
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
        if(findItemInItemset(itemset2, production, dotPosition) == xyItemNull) {
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

// Move incomming Iedges from itemset1 to the corresponding core rules in
// itemset2.  This happens when merging itemsets with identical cores.
static void moveIedgeDestsToItemset(xyItemset itemset1, xyItemset itemset2) {
    xyItem item1, item2;
    xyForeachItemsetItem(itemset1, item1) {
        utAssert(xyItemGetFirstOutIedge(item1) == xyIedgeNull);
        xyIedge iedge;
        xySafeForeachItemInIedge(item1, iedge) {
            item2 = xyIedgeGetFromItem(iedge);
            if(xyItemGetItemset(item2) != itemset1) {
                xyItemRemoveInIedge(item1, iedge);
                xyItem destItem = findItemInItemset(itemset2, xyItemGetProduction(item1),
                    xyItemGetDotPosition(item1));
                utAssert(destItem != xyItemNull);
                printf("Moving iedge\n");
                xyItemAppendInIedge(destItem, iedge);
            }
        } xyEndSafeItemOutIedge;
    } xyEndItemsetItem;
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
            printf("Merging itemset %u into %u\n", xyItemset2Index(destItemset), xyItemset2Index(oldItemset));
            xyItemsetRemoveInTransition(destItemset, transition);
            xyItemsetAppendInTransition(oldItemset, transition);
            moveIedgeDestsToItemset(destItemset, oldItemset);
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
        buildNewItemsetsFromItemset(parser, itemset);
        mergeItemsetsWithIdenticalCores(itemset);
    } xyEndParserItemset;
}

// Add the titems from tset1 to tset2.
static void addTsetToTset(xyTset tset1, xyTset tset2) {
    xyTitem titem;
    xyForeachTsetTitem(tset1, titem) {
        xyTitemCreate(tset2, xyTitemGetMtoken(titem));
    } xyEndTsetTitem;
}

// Forward declaration for double-recursion.
static void computeMtokenFirstTset(xyMtoken mtoken);

// Update the FIRST tset with the first elements reachable in the production.
static void updateFirstTsetWithProduction(xyTset tset, xyProduction production) {
    xyToken token;
    xyForeachProductionToken(production, token) {
        xyMtoken mtoken = xyTokenGetMtoken(token);
        if(xyMtokenGetType(mtoken) != XY_NONTERM) {
            xyTitemCreate(tset, mtoken);
            return;
        }
        xyTset ntset = xyMtokenGetFirstTset(mtoken);
        if(ntset == xyTsetNull) {
            computeMtokenFirstTset(mtoken);
            ntset = xyMtokenGetFirstTset(mtoken);
        }
        addTsetToTset(ntset, tset);
        if(!xyTsetHasEmpty(ntset)) {
            return;
        }
    } xyEndProductionToken;
    xyTsetSetHasEmpty(tset, true);
}

// Compute the tset for the mtoken.  If followed by a token who's mtoken is not
// yet computed, compute that one first.
static void computeMtokenFirstTset(xyMtoken mtoken) {
    xyTset tset = xyTsetAlloc();
    xyMtokenSetFirstTset(mtoken, tset);
    xyParser parser = xyMtokenGetParser(mtoken);
    xyRule rule = xyParserFindRule(parser, xyMtokenGetSym(mtoken));
    xyProduction production;
    xyForeachRuleProduction(rule, production) {
        updateFirstTsetWithProduction(tset, production);
    } xyEndRuleProduction;
}

// Compute the tsets of tokens that can be parsed first for each nonterminal.
static void computeFirstTsets(xyParser parser) {
    xyMtoken mtoken;
    xyForeachParserMtoken(parser, mtoken) {
        if(xyMtokenGetType(mtoken) == XY_NONTERM && xyMtokenGetFirstTset(mtoken) == xyTsetNull) {
            computeMtokenFirstTset(mtoken);
        }
    } xyEndParserMtoken;
}

// Add the tset item to the destination if it does not already exist.  Return
// true if it did not exist.
static bool addTsetMtoken(xyTset tset, xyMtoken mtoken) {
    xyTitem prevTitem = xyTsetFindTitem(tset, mtoken);
    if(prevTitem == xyTitemNull) {
        xyTitemCreate(tset, mtoken);
        return true;
    }
    return false;
}

// Add the item to the updated list of items if it is not already there.
static void addItemToUpdateList(xyItem item) {
    if(xyItemInUpdateList(item)) {
        return;
    }
    xyItemSetInUpdateList(item, true);
    xyParser parser = xyItemsetGetParser(xyItemGetItemset(item));
    xyParserAppendUpdatedItem(parser, item);
}

// Copy lookaheads from tset1 to tset2, and if any are new, return true.
static bool copyTsetLookaheads(xyTset tset1, xyTset tset2) {
    bool addedTitem = false;
    xyTitem titem;
    xyForeachTsetTitem(tset1, titem) {
        xyMtoken mtoken = xyTitemGetMtoken(titem);
        addedTitem |= addTsetMtoken(tset2, mtoken);
    } xyEndTsetTitem;
    return addedTitem;
}

// Add tokes fron the "first" tokens seen from the position in item1 to tset.
// If we see a nonterminal that can be empty, continue to the next token.  If
// we get to the end, add item's entire lookahead set.  Return true if we added
// anything.
static bool addLookaheadsFromFirst(xyItem item, xyTset tset) {
    uint32 pos = xyItemGetDotPosition(item) + 1;
    bool addedSomething = false;
    xyProduction production = xyItemGetProduction(item);
    uint32 numTokens = xyProductionGetUsedToken(production);
    while(pos < numTokens) {
        xyToken token = xyProductionGetiToken(production, pos);
        xyMtoken mtoken = xyTokenGetMtoken(token);
        if(xyMtokenGetType(mtoken) != XY_NONTERM) {
            addedSomething |= addTsetMtoken(tset, mtoken);
            return addedSomething;
        }
        xyTset firstTset = xyMtokenGetFirstTset(mtoken);
        addedSomething |= copyTsetLookaheads(firstTset, tset);
        if(!xyTsetHasEmpty(firstTset)) {
            return addedSomething;
        }
        pos++;
    }
    addedSomething |= copyTsetLookaheads(xyItemGetLookaheadTset(item), tset);
    return addedSomething;
}

// Propagate changes to the lookahead set of this item.
static void propagateLookaheads(xyItem item) {
    xyItemset itemset = xyItemGetItemset(item);
    xyTset sourceTset = xyItemGetLookaheadTset(item);
    xyIedge iedge;
    xyForeachItemOutIedge(item, iedge) {
        xyItem nitem = xyIedgeGetToItem(iedge);
        xyTset destTset = xyItemGetLookaheadTset(nitem);
        if(destTset == xyTsetNull) {
            destTset = xyTsetAlloc();
            xyItemSetLookaheadTset(nitem, destTset);
        }
        bool updatedLookaheads;
        if(xyItemGetItemset(nitem) != itemset) {
            // Just copy over lookaheads
            updatedLookaheads = copyTsetLookaheads(sourceTset, destTset);
        } else {
            updatedLookaheads = addLookaheadsFromFirst(item, destTset);
        }
        if(updatedLookaheads) {
            addItemToUpdateList(nitem);
        }
    } xyEndItemOutIedge;
}

// Compute the tokens that can follow after any given reduction.
static void computeLookaheadSets(xyParser parser) {
    xyItem item = xyParserGetFirstUpdatedItem(parser);
    while(item != xyItemNull) {
        xyParserRemoveUpdatedItem(parser, item);
        xyItemSetInUpdateList(item, false);
        propagateLookaheads(item);
        item = xyParserGetFirstUpdatedItem(parser);
    }
}

// Build actions for the state from the itemset.
static void buildStateActions(agState state) {
    xyItemset itemset = xyStateGetItemset(state);
    xyTransition transition;
    xyForeachItemsetOutTransition(itemset, transition) {
        xyAction action = xyActionCreate(XY_GOTO, state, xyTransitionGetMtoken(transition));
        xyActionSetDestState(action, 
    } xyEndItemsetOutTransition;
}

// Build an AGTable from the itemsets.
static xyAGTable buildAGTable(xyParser parser) {
    xyAGTable agtable = xyAGTableAlloc();
    xyItemset itemset;
    xyForeachParserItemset(parser, itemset) {
        xyState state = xyStateAlloc();
        xyItemsetInsertState(itemset, state);
        xyAGTableAppendState(agtable, state);
    } xyEndParserItemset;
    xyForeachAGTableState(agtable, state) {
        buildStateActions(state);
    } xyEndAGTableState;
    return agtable;
}

// Build all the item sets.
xyAGTable xyBuildAGTable(xyParser parser) {
    xyRule goal = xyParserGetFirstRule(parser);
    xyItemset goalSet = xyItemsetCreate(parser);
    addRuleToItemset(goalSet, xyItemNull, goal, true);
    computeLR0Sets(parser);
    computeFirstTsets(parser);
    addEOFTokenToLookaheads(goal);
    computeLookaheadSets(parser);
    xyPrintParser(parser);
    return buildAGTable(parser);
}
