#include "parse_int.h"

FILE *paFile;
uint32 paLinenum;
xyParser paCurrentParser;

// Print out the stack.
static void printStack(xyStateArray states, xyTokenArray tokens) {
    putchar('[');
    bool firstTime = true;
    xyState state;
    xyForeachStateArrayState(states, state) {
        if(!firstTime) {
            putchar(' ');
        }
        firstTime = false;
        printf("%u", xyStateGetParserIndex(state));
    } xyEndStateArrayState;
    printf("] [");
    firstTime = true;
    for(uint32 i = 0; i < xyTokenArrayGetUsedToken(tokens); i++) {
        xyToken token = xyTokenArrayGetiToken(tokens, i);
        if(!firstTime) {
            putchar(' ');
        }
        firstTime = false;
        if(token == xyTokenNull) {
            printf("null");
        } else {
            xyPrintToken(token);
        }
    }
    printf("]\n");
}

// Push a state onto the stack.
static inline xyState push(xyStateArray states, xyTokenArray tokens, xyState state, xyToken token) {
    xyStateArrayAppendState(states, state);
    xyTokenArrayAppendToken(tokens, token);
    return state;
}

// Pop statesToPop states off the stack.
static void pop(xyStateArray states, xyTokenArray tokens, uint32 statesToPop) {
    xyStateArraySetUsedState(states, xyStateArrayGetUsedState(states) - statesToPop);
    uint32 start = xyTokenArrayGetUsedToken(tokens) - statesToPop;
    for(uint32 i = 0; i < statesToPop; i++) {
        xyToken token = xyTokenArrayGetiToken(tokens, start + i);
        if(token != xyTokenNull) {
            xyTokenDestroy(token);
        }
    }
    xyTokenArraySetUsedToken(tokens, xyTokenArrayGetUsedToken(tokens) - statesToPop);
}

// Return the top state on the stack.
static inline xyState top(xyStateArray states) {
    return xyStateArrayGetiState(states, xyStateArrayGetUsedState(states) - 1);
}

// Forward declaration for recursion.
static xyToken executeMap(xyMap map, xyTokenArray tokens, uint32 statesToPop, xyToken token);

// Execute a concatenation map.
static xyToken executeConcatMap(xyMap map, xyTokenArray tokens, uint32 statesToPop, xyToken token) {
    xyMap leftMap = xyMapGetFirstMap(map);
    xyMap rightMap = xyMapGetNextMapMap(leftMap);
    xyToken leftToken = executeMap(leftMap, tokens, statesToPop, token);
    xyToken rightToken = executeMap(rightMap, tokens, statesToPop, token);
    if(leftToken == xyTokenNull || rightToken == xyTokenNull ||
            xyTokenGetType(leftToken) != XY_LIST || xyTokenGetType(rightToken) != XY_LIST) {
        xyError(token, "Expected two list tokens in concat map");
    }
    xyList leftList = xyTokenGetListVal(leftToken);
    xyList rightList = xyTokenGetListVal(rightToken);
    xyForeachListToken(rightList, token) {
        xyListRemoveToken(rightList, token);
        xyListAppendToken(leftList, token);
    } xyEndListToken;
    xyListDestroy(rightList);
    return leftToken;
}

// Execute an append map.
static xyToken executeAppendMap(xyMap map, xyTokenArray tokens, uint32 statesToPop, xyToken token) {
    xyMap leftMap = xyMapGetFirstMap(map);
    xyMap rightMap = xyMapGetNextMapMap(leftMap);
    xyToken leftToken = executeMap(leftMap, tokens, statesToPop, token);
    xyToken rightToken = executeMap(rightMap, tokens, statesToPop, token);
    if(leftToken == xyTokenNull || xyTokenGetType(leftToken) != XY_LIST) {
        xyError(token, "Expected list left token in append map");
    }
    if(rightToken != xyTokenNull) {
        xyListAppendToken(xyTokenGetListVal(leftToken), rightToken);
    }
    return leftToken;
}

// Execute a list map.
static xyToken executeListMap(xyMap map, xyTokenArray tokens, uint32 statesToPop, xyToken token) {
    xyList list = xyListAlloc();
    xyMap child;
    uint32 minLinenum = 0;
    xyForeachMapMap(map, child) {
        xyToken value = executeMap(child, tokens, statesToPop, token);
        if(value != xyTokenNull) {
            xyListAppendToken(list, value);
            uint32 linenum = xyTokenGetLinenum(value);
            if(linenum != 0 && (minLinenum == 0 || linenum < minLinenum)) {
                minLinenum = linenum;
            }
        }
    } xyEndMapMap;
    return xyListTokenCreate(paCurrentParser, list, minLinenum);
}

// Execute a token map.
static xyToken executeTokenMap(xyMap map, xyTokenArray tokens, uint32 statesToPop) {
    uint32 start = xyTokenArrayGetUsedToken(tokens) - statesToPop;
    xyToken token = xyTokenArrayGetiToken(tokens,  start + xyMapGetPosition(map));
    xyTokenArraySetiToken(tokens, start + xyMapGetPosition(map), xyTokenNull);
    return token;
}

// Execute a default map, by just putting all the tokens in a list.
static xyToken executeDefaultMap(xyTokenArray tokens, uint32 statesToPop, xyToken token) {
    uint32 start = xyTokenArrayGetUsedToken(tokens) - statesToPop;
    if(statesToPop == 1) {
        xyToken value = xyTokenArrayGetiToken(tokens, start);
        xyTokenArraySetiToken(tokens, start, xyTokenNull);
        return value;
    } else if(statesToPop == 0) {
        return xyTokenNull;
    }
    uint32 minLinenum = 0;
    xyList list = xyListAlloc();
    for(uint32 i = 0; i < statesToPop; i++) {
        xyToken value = xyTokenArrayGetiToken(tokens, start + i);
        if(value != xyTokenNull) {
            xyListAppendToken(list, value);
        }
        xyTokenArraySetiToken(tokens, start + i, xyTokenNull);
        uint32 linenum = xyTokenGetLinenum(value);
        if(linenum != 0 && (minLinenum == 0 || linenum < minLinenum)) {
            minLinenum = linenum;
        }
    }
    return xyListTokenCreate(paCurrentParser, list, minLinenum);
}

// Execute a map expression to combinethe tokens on the top of the stack.
static xyToken executeMap(xyMap map, xyTokenArray tokens, uint32 statesToPop, xyToken token) {
    if(map == xyMapNull) {
        return executeDefaultMap(tokens, statesToPop, token);
    }
    xyToken value;
    switch(xyMapGetType(map)) {
    case XY_MAP_CONCAT:
        value = executeConcatMap(map, tokens, statesToPop, token);
        break;
    case XY_MAP_APPEND:
        value = executeAppendMap(map, tokens, statesToPop, token);
        break;
    case XY_MAP_LIST:
        value = executeListMap(map, tokens, statesToPop, token);
        break;
    case XY_MAP_TOKEN:
        value = executeTokenMap(map, tokens, statesToPop);
        break;
    case XY_MAP_KEYWORD:
        value = xyKeywordTokenCreate(paCurrentParser, xyMapGetSym(map), xyTokenGetLinenum(token));
        break;
    case XY_MAP_NULL:
        value = xyNullTokenCreate(paCurrentParser, xyTokenGetLinenum(token));
        break;
    default:
        utExit("Unknown map type");
    }
    // applyMapAttributes(map, value);
    return value;
}

// Parse input tokens util we accept, or find an error.
static xyToken parseUntilAccept(xyParser parser, xyStateArray states, xyTokenArray tokens) {
    xyState state = top(states);
    xyToken token = paLex();
    while(true) {
        xyPrintToken(token);
        printStack(states, tokens);
        xyMtoken mtoken = xyTokenGetMtoken(token);
        xyAction action = xyStateFindAction(state, mtoken);
        if(action == xyActionNull) {
            xyError(token, "Syntax error");
        }
        xyMtoken reduceMtoken;
        xyToken reducedToken;
        xyAction gotoAction;
        uint32 statesToPop;
        switch(xyActionGetType(action)) {
        case XY_ACCEPT:
            return xyTokenArrayGetiToken(tokens, 1);
        case XY_SHIFT:
            state = push(states, tokens, xyActionGetDestState(action), token);
            token = paLex();
            break;
        case XY_REDUCE:
            reduceMtoken = xyActionGetReduceMtoken(action);
            printf("Reducing %s\n", xyMtokenGetName(reduceMtoken));
            statesToPop = xyActionGetStatesToPop(action);
            reducedToken = executeMap(xyActionGetMap(action), tokens, statesToPop, token);
            pop(states, tokens, statesToPop);
            state = top(states);
            gotoAction = xyStateFindAction(state, reduceMtoken);
            state = xyActionGetDestState(gotoAction);
            push(states, tokens, state, reducedToken);
            break;
        default:
            utExit("Unknown action type");
        }
    }
}

// Parse the input file using the action-goto table.
xyToken paParse(FILE *file, xyParser parser) {
    paFile = file;
    utf8Start();
    paCurrentParser = parser;
    paLexerStart();
    xyStateArray states = xyStateArrayAlloc();
    xyTokenArray tokens = xyTokenArrayAlloc();
    push(states, tokens, xyParserGetiState(parser, 0), xyTokenNull);
    xyToken token = parseUntilAccept(parser, states, tokens);
    xyTokenArrayFree(tokens);
    xyStateArrayFree(states);
    paLexerStop();
    utf8Stop();
    return token;
}
