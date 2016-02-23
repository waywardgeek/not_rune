/*
  The algorithm here is mostly from these two sites:

  https://lambda.uta.edu/cse5317/notes/node20.html
  http://web.cs.dal.ca/~sjackson/lalr1.html

  The lambda.uta.edu article mentioned simply building the LR(0) set, and then
  adding the "lookahead" sets as a post-process, which is done here.  More
  details of how to make this all work is found on the other site.
*/
#include "parsegen_int.h"

// Print the item.
void xpPrintItem(xpItem item) {
    printf("%u: ", xpItem2Index(item));
    xpProduction production = xpItemGetProduction(item);
    xpRule rule = xpProductionGetRule(production);
    if(!xpItemCore(item)) {
        printf("+ ");
    }
    printf("%s ->", xpRuleGetName(rule));
    xpToken token;
    uint32 position = 0;
    uint32 dotPosition = xpItemGetDotPosition(item);
    xpForeachProductionToken(production, token) {
        if(position == dotPosition) {
            printf(" .");
        }
        putchar(' ');
        xpPrintToken(token);
        position++;
    } xpEndProductionToken;
    if(position == dotPosition) {
        printf(" .");
    }
    xpTset lookaheads = xpItemGetLookaheadTset(item);
    if(lookaheads != xpTsetNull) {
        printf(" [");
        xpTentry tentry;
        bool firstTime = true;
        xpForeachTsetTentry(lookaheads, tentry) {
            if(!firstTime) {
                putchar(' ');
            }
            firstTime = false;
            printf("%s", xyMtokenGetName(xpTentryGetMtoken(tentry)));
        } xpEndTsetTentry;
        printf("]");
    }
    bool firstTime = true;
    xpIedge iedge;
    xpForeachItemOutIedge(item, iedge) {
        if(firstTime) {
            printf(" :");
            firstTime = false;
        }
        printf(" %u", xpItem2Index(xpIedgeGetToItem(iedge)));
    } xpEndItemOutIedge;
    putchar('\n');
}

// Print the itemset.
void xpPrintItemset(xpItemset itemset) {
    printf("Itemset %u\n", xpItemset2Index(itemset));
    xpItem item;
    xpForeachItemsetItem(itemset, item) {
        printf("    ");
        xpPrintItem(item);
    } xpEndItemsetItem;
}

// Create a new itemset.
xpItemset xpItemsetCreate(xyParser parser, bool ignoreNewlines) {
    xpItemset itemset = xpItemsetAlloc();
    xpItemsetSetIgnoreNewlines(itemset, ignoreNewlines);
    xpParserAppendItemset(parser, itemset);
    return itemset;
}

// Add the production to the itemset.
xpItem xpItemCreate(xpItemset itemset, xpItem prevItem, xpProduction production, uint32 position, bool core) {
    xpItem item = xpItemAlloc();
    xpItemSetDotPosition(item, position);
    xpItemSetCore(item, core);
    xpProductionAppendItem(production, item);
    xpItemsetAppendItem(itemset, item);
    return item;
}

// Create a transition object between itemsets.
xpTransition xpTransitionCreate(xyMtoken mtoken, xpItemset fromItemset, xpItemset toItemset) {
    xpTransition transition = xpTransitionAlloc();
    xpMtokenAppendTransition(mtoken, transition);
    xpItemsetAppendOutTransition(fromItemset, transition);
    xpItemsetAppendInTransition(toItemset, transition);
    return transition;
}

// Create a new dependency edge between two items.
xpIedge xpIedgeCreate(xpItem fromItem, xpItem toItem, bool closure) {
    xpIedge iedge = xpIedgeAlloc();
    xpIedgeSetClosure(iedge, closure);
    xpItemAppendOutIedge(fromItem, iedge);
    xpItemAppendInIedge(toItem, iedge);
    return iedge;
}

// Create a new item in a token set.
xpTentry xpTentryCreate(xpTset tset, xyMtoken mtoken) {
    xpTentry tentry = xpTsetFindTentry(tset, mtoken);
    if(tentry != xpTentryNull) {
        // Already added it
        return tentry;
    }
    tentry = xpTentryAlloc();
    xpTentrySetMtoken(tentry, mtoken);
    xpTsetInsertTentry(tset, tentry);
    return tentry;
}

// Add the EOF token to each production in the rule.  This is done just for the
// first goal rule.
static void addEOFTokenToLookaheads(xpRule rule) {
    xyParser parser = xpRuleGetParser(rule);
    xyMtoken eofMtoken = xyMtokenCreate(parser, XY_TOK_EOF, utSymNull);
    xpProduction production;
    xpForeachRuleProduction(rule, production) {
        xpItem item;
        xpForeachProductionItem(production, item) {
            xpTset tset = xpTsetAlloc();
            xpItemSetLookaheadTset(item, tset);
            xpTentryCreate(tset, eofMtoken);
            xpParserAppendUpdatedItem(parser, item);
        } xpEndProductionItem;
    } xpEndRuleProduction;
}

// Determine if the item is already in the itemset.
static xpItem findItemInItemset(xpItemset itemset, xpProduction production, uint32 dotPosition) {
    xpItem item;
    xpForeachItemsetItem(itemset, item) {
        if(xpItemGetProduction(item) == production && xpItemGetDotPosition(item) == dotPosition) {
            return item;
        }
    } xpEndItemsetItem;
    return xpItemNull;
}

// Add the rule's productions to the itemset.
static void addRuleToItemset(xpItemset itemset, xpItem prevItem, xpRule rule, bool core) {
    xpProduction production;
    xpForeachRuleProduction(rule, production) {
        xpItem item = findItemInItemset(itemset, production, 0);
        if(item == xpItemNull) {
            item = xpItemCreate(itemset, prevItem, production, 0, core);
        }
        if(prevItem != xpItemNull) {
            xpIedgeCreate(prevItem, item, true);
        }
    } xpEndRuleProduction;
}

// Add additional items to the itemset until there are no productions that have
// not been used to expand the itemset.  A production can only be included
// once.
static void computeClosure(xyParser parser, xpItemset itemset) {
    xpItem item;
    xpForeachItemsetItem(itemset, item) {
        xpProduction production = xpItemGetProduction(item);
        uint32 position = xpItemGetDotPosition(item);
        if(position < xpProductionGetUsedToken(production)) {
            xpToken token = xpProductionGetiToken(production, position);
            xyMtoken mtoken = xpTokenGetMtoken(token);
            if(xyMtokenGetType(mtoken) == XY_TOK_NONTERM) {
                xpRule rule = xpMtokenGetRule(mtoken);
                if(rule == xpRuleNull) {
                    utError("Undefined non-terminal %s", xyMtokenGetName(mtoken));
                }
                addRuleToItemset(itemset, item, rule, false);
            }
        }
    } xpEndItemsetItem;
}

// Build new itemsets that we can transition to from this one.
static void buildNewItemsetsFromItemset(xyParser parser, xpItemset itemset) {
    //printf("Building transitions from itemset:");
    //xpPrintItemset(itemset);
    xpItem item;
    xpForeachItemsetItem(itemset, item) {
        uint32 position = xpItemGetDotPosition(item);
        xpProduction production = xpItemGetProduction(item);
        if(position < xpProductionGetUsedToken(production)) {
            xpToken token = xpProductionGetiToken(production, position);
            xyMtoken mtoken = xpTokenGetMtoken(token);
            xpTransition transition = xpItemsetFindOutTransition(itemset, mtoken);
            xpItemset destItemset;
            if(transition == xpTransitionNull) {
                //printf("New itemset for ");
                //xpPrintItem(item);
                destItemset = xpItemsetCreate(parser, xpProductionIgnoreNewlines(production));
                transition = xpTransitionCreate(mtoken, itemset, destItemset);
            } else {
                //printf("Found existing itemset for ");
                //xpPrintItem(item);
                destItemset = xpTransitionGetToItemset(transition);
            }
            xpItem destItem = findItemInItemset(destItemset, production, position + 1);
            if(destItem == xpItemNull) {
                destItem = xpItemCreate(destItemset, item, production, position + 1, true);
            }
            xpIedgeCreate(item, destItem, false);
        }
    } xpEndItemsetItem;
}

// Determine if the itemsets have the same core rules, with the same dot positions.
static bool itemsetsHaveSameCore(xpItemset itemset1, xpItemset itemset2) {
    xpItem item;
    uint32 numCores1 = 0;
    for(item = xpItemsetGetFirstItem(itemset1); item != xpItemNull && xpItemCore(item);
            item = xpItemGetNextItemsetItem(item)) {
        xpProduction production = xpItemGetProduction(item);
        uint32 dotPosition = xpItemGetDotPosition(item);
        if(findItemInItemset(itemset2, production, dotPosition) == xpItemNull) {
            return false;
        }
        numCores1++;
    }
    uint32 numCores2 = 0;
    for(item = xpItemsetGetFirstItem(itemset2); item != xpItemNull && xpItemCore(item);
            item = xpItemGetNextItemsetItem(item)) {
        numCores2++;
    }
    return numCores1 == numCores2;
}

// Find an existing itemset with the same core productions.
static xpItemset findExistingIdenticalItemset(xpItemset newItemset) {
    xpProduction firstProduction = xpItemGetProduction(xpItemsetGetFirstItem(newItemset));
    xpItem item;
    xpForeachProductionItem(firstProduction, item) {
        xpItemset oldItemset = xpItemGetItemset(item);
        if(oldItemset != newItemset && itemsetsHaveSameCore(newItemset, oldItemset)) {
            return oldItemset;
        }
    } xpEndProductionItem;
    return xpItemsetNull;
}

// Move incomming Iedges from itemset1 to the corresponding core rules in
// itemset2.  This happens when merging itemsets with identical cores.
static void moveIedgeDestsToItemset(xpItemset itemset1, xpItemset itemset2) {
    xpItem item1, item2;
    xpForeachItemsetItem(itemset1, item1) {
        utAssert(xpItemGetFirstOutIedge(item1) == xpIedgeNull);
        xpIedge iedge;
        xpSafeForeachItemInIedge(item1, iedge) {
            item2 = xpIedgeGetFromItem(iedge);
            if(xpItemGetItemset(item2) != itemset1) {
                xpItemRemoveInIedge(item1, iedge);
                xpItem destItem = findItemInItemset(itemset2, xpItemGetProduction(item1),
                    xpItemGetDotPosition(item1));
                utAssert(destItem != xpItemNull);
                printf("Moving iedge\n");
                xpItemAppendInIedge(destItem, iedge);
            }
        } xpEndSafeItemOutIedge;
    } xpEndItemsetItem;
}

// If outgoing transitions lead to itemsets that have identical cores to
// existing itemsets, merge them.
static void mergeItemsetsWithIdenticalCores(xpItemset itemset) {
    xpTransition transition;
    xpItemset destItemset;
    xpForeachItemsetOutTransition(itemset, transition) {
        destItemset = xpTransitionGetToItemset(transition);
        xpItemset oldItemset = findExistingIdenticalItemset(destItemset);
        if(oldItemset != xpItemsetNull) {
            printf("Merging itemset %u into %u\n", xpItemset2Index(destItemset), xpItemset2Index(oldItemset));
            xpItemsetRemoveInTransition(destItemset, transition);
            xpItemsetAppendInTransition(oldItemset, transition);
            moveIedgeDestsToItemset(destItemset, oldItemset);
            xpItemsetDestroy(destItemset);
        }
    } xpEndItemsetOutTransition;
}

// Compute all the itemsets, starting with just the goal itemset, by computing
// closures and then new sets.  LR(0) sets are the same if they are generated
// from the same items.
static void computeLR0Sets(xyParser parser) {
    xpItemset itemset;
    xpForeachParserItemset(parser, itemset) {
        // Note that we append itemsets to the parser in this loop.
        computeClosure(parser, itemset);
        buildNewItemsetsFromItemset(parser, itemset);
        mergeItemsetsWithIdenticalCores(itemset);
    } xpEndParserItemset;
}

// Add the tentrys from tset1 to tset2.
static void addTsetToTset(xpTset tset1, xpTset tset2) {
    xpTentry tentry;
    xpForeachTsetTentry(tset1, tentry) {
        xpTentryCreate(tset2, xpTentryGetMtoken(tentry));
    } xpEndTsetTentry;
}

// Forward declaration for double-recursion.
static void computeMtokenFirstTset(xyMtoken mtoken);

// Update the FIRST tset with the first elements reachable in the production.
static void updateFirstTsetWithProduction(xpTset tset, xpProduction production) {
    xpToken token;
    xpForeachProductionToken(production, token) {
        xyMtoken mtoken = xpTokenGetMtoken(token);
        if(xyMtokenGetType(mtoken) != XY_TOK_NONTERM) {
            xpTentryCreate(tset, mtoken);
            return;
        }
        xpTset ntset = xpMtokenGetFirstTset(mtoken);
        if(ntset == xpTsetNull) {
            computeMtokenFirstTset(mtoken);
            ntset = xpMtokenGetFirstTset(mtoken);
        }
        addTsetToTset(ntset, tset);
        if(!xpTsetHasEmpty(ntset)) {
            return;
        }
    } xpEndProductionToken;
    xpTsetSetHasEmpty(tset, true);
}

// Compute the tset for the mtoken.  If followed by a token who's mtoken is not
// yet computed, compute that one first.
static void computeMtokenFirstTset(xyMtoken mtoken) {
    xpTset tset = xpTsetAlloc();
    xpMtokenSetFirstTset(mtoken, tset);
    xpRule rule = xpMtokenGetRule(mtoken);
    xpProduction production;
    xpForeachRuleProduction(rule, production) {
        updateFirstTsetWithProduction(tset, production);
    } xpEndRuleProduction;
}

// Compute the tsets of tokens that can be parsed first for each nonterminal.
static void computeFirstTsets(xyParser parser) {
    xyMtoken mtoken;
    xyForeachParserMtoken(parser, mtoken) {
        if(xyMtokenGetType(mtoken) == XY_TOK_NONTERM && xpMtokenGetFirstTset(mtoken) == xpTsetNull) {
            computeMtokenFirstTset(mtoken);
        }
    } xyEndParserMtoken;
}

// Add the tset item to the destination if it does not already exist.  Return
// true if it did not exist.
static bool addTsetMtoken(xpTset tset, xyMtoken mtoken) {
    xpTentry prevTentry = xpTsetFindTentry(tset, mtoken);
    if(prevTentry == xpTentryNull) {
        xpTentryCreate(tset, mtoken);
        return true;
    }
    return false;
}

// Add the item to the updated list of items if it is not already there.
static void addItemToUpdateList(xpItem item) {
    if(xpItemInUpdateList(item)) {
        return;
    }
    xpItemSetInUpdateList(item, true);
    xyParser parser = xpItemsetGetParser(xpItemGetItemset(item));
    xpParserAppendUpdatedItem(parser, item);
}

// Copy lookaheads from tset1 to tset2, and if any are new, return true.
static bool copyTsetLookaheads(xpTset tset1, xpTset tset2) {
    bool addedTentry = false;
    xpTentry tentry;
    xpForeachTsetTentry(tset1, tentry) {
        xyMtoken mtoken = xpTentryGetMtoken(tentry);
        addedTentry |= addTsetMtoken(tset2, mtoken);
    } xpEndTsetTentry;
    return addedTentry;
}

// Add tokes fron the "first" tokens seen from the position in item1 to tset.
// If we see a nonterminal that can be empty, continue to the next token.  If
// we get to the end, add item's entire lookahead set.  Return true if we added
// anything.
static bool addLookaheadsFromFirst(xpItem item, xpTset tset) {
    uint32 pos = xpItemGetDotPosition(item) + 1;
    bool addedSomething = false;
    xpProduction production = xpItemGetProduction(item);
    uint32 numTokens = xpProductionGetUsedToken(production);
    while(pos < numTokens) {
        xpToken token = xpProductionGetiToken(production, pos);
        xyMtoken mtoken = xpTokenGetMtoken(token);
        if(xyMtokenGetType(mtoken) != XY_TOK_NONTERM) {
            addedSomething |= addTsetMtoken(tset, mtoken);
            return addedSomething;
        }
        xpTset firstTset = xpMtokenGetFirstTset(mtoken);
        addedSomething |= copyTsetLookaheads(firstTset, tset);
        if(!xpTsetHasEmpty(firstTset)) {
            return addedSomething;
        }
        pos++;
    }
    addedSomething |= copyTsetLookaheads(xpItemGetLookaheadTset(item), tset);
    return addedSomething;
}

// Propagate changes to the lookahead set of this item.
static void propagateLookaheads(xpItem item) {
    xpTset sourceTset = xpItemGetLookaheadTset(item);
    xpIedge iedge;
    xpForeachItemOutIedge(item, iedge) {
        xpItem nitem = xpIedgeGetToItem(iedge);
        xpTset destTset = xpItemGetLookaheadTset(nitem);
        if(destTset == xpTsetNull) {
            destTset = xpTsetAlloc();
            xpItemSetLookaheadTset(nitem, destTset);
        }
        bool updatedLookaheads;
        if(xpIedgeClosure(iedge)) {
            updatedLookaheads = addLookaheadsFromFirst(item, destTset);
        } else {
            updatedLookaheads = copyTsetLookaheads(sourceTset, destTset);
        }
        if(updatedLookaheads) {
            addItemToUpdateList(nitem);
        }
    } xpEndItemOutIedge;
}

// Compute the tokens that can follow after any given reduction.
static void computeLookaheadSets(xyParser parser) {
    xpItem item = xpParserGetFirstUpdatedItem(parser);
    while(item != xpItemNull) {
        xpParserRemoveUpdatedItem(parser, item);
        xpItemSetInUpdateList(item, false);
        propagateLookaheads(item);
        item = xpParserGetFirstUpdatedItem(parser);
    }
}

// Build actions for the state from the itemset.
static bool buildStateActions(xyState state) {
    bool passed = true;
    xpItemset itemset = xpStateGetItemset(state);
    xpTransition transition;
    xpForeachItemsetOutTransition(itemset, transition) {
        xyState destState = xpItemsetGetState(xpTransitionGetToItemset(transition));
        xyMtoken mtoken = xpTransitionGetMtoken(transition);
        if(xyMtokenGetType(mtoken) == XY_TOK_NONTERM) {
            passed &= xyGotoActionCreate(state, mtoken, destState) != xyActionNull;
        } else {
            passed &= xyShiftActionCreate(state, mtoken, destState) != xyActionNull;
        }
    } xpEndItemsetOutTransition;
    xyParser parser = xpItemsetGetParser(itemset);
    xpItem item;
    xpForeachItemsetItem(itemset, item) {
        xpProduction production = xpItemGetProduction(item);
        xpRule rule = xpProductionGetRule(production);
        uint32 numTokens = xpProductionGetUsedToken(production);
        if(xpItemGetDotPosition(item) == numTokens) {
            xpTentry tentry;
            xyMtoken reduceMtoken = xpRuleGetMtoken(rule);
            xyMap map = xpProductionGetMap(production);
            xpForeachTsetTentry(xpItemGetLookaheadTset(item), tentry) {
                xyMtoken mtoken = xpTentryGetMtoken(tentry);
                if(rule != xpParserGetFirstRule(parser)) {
                    passed &= xyReduceActionCreate(state, mtoken, reduceMtoken, numTokens, map) !=
                        xyActionNull;
                } else {
                    passed &= xyAcceptActionCreate(state, mtoken) != xyActionNull;
                }
            } xpEndTsetTentry;
        }
    } xpEndItemsetItem;
    return passed;
}

// Build an Parser from the itemsets.
static bool buildParserActionGotoTable(xyParser parser) {
    xpItemset itemset;
    xpForeachParserItemset(parser, itemset) {
        xyState state = xyStateAlloc();
        xyStateSetIgnoreNewlines(state, xpItemsetIgnoreNewlines(itemset));
        xpItemsetInsertState(itemset, state);
        xyParserAppendState(parser, state);
    } xpEndParserItemset;
    bool passed = true;
    xyState state;
    xyForeachParserState(parser, state) {
        passed = buildStateActions(state);
    } xyEndParserState;
    return passed;
}

// Build all the item sets.  Return true if there are no shift/reduce or reduce/reduce errors.
bool xpBuildParserActionGotoTable(xyParser parser) {
    xpRule goal = xpParserGetFirstRule(parser);
    xpItemset goalSet = xpItemsetCreate(parser, false);
    addRuleToItemset(goalSet, xpItemNull, goal, true);
    computeLR0Sets(parser);
    computeFirstTsets(parser);
    addEOFTokenToLookaheads(goal);
    computeLookaheadSets(parser);
    xpPrintParser(parser);
    return buildParserActionGotoTable(parser);
}
