#include "parse_int.h"

FILE *paFile;
uint32 paLineNum;

static utSym paEOFSym, paNEWLINESym;

// Print out the stack.
static void printStack(xyStateArray states, xyValueArray values) {
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
    for(uint32 i = 0; i < xyValueArrayGetUsedValue(values); i++) {
        xyValue value = xyValueArrayGetiValue(values, i);
        if(!firstTime) {
            putchar(' ');
        }
        firstTime = false;
        if(value == xyValueNull) {
            printf("null");
        } else {
            xyPrintValue(value);
        }
    }
    printf("]\n");
}

// Build a value from a token.
static xyValue buildTokenValue(paToken token) {
    xyMtoken mtoken = paTokenGetMtoken(token);
    utSym sym = utSymNull;
    if(mtoken != xyMtokenNull) {
        sym = xyMtokenGetSym(mtoken);
    }
    switch(paTokenGetType(token)) {
    case XY_TOK_KEYWORD:
        return xySymValueCreate(sym);
    case XY_TOK_INT:
        return xyUintValueCreate(paTokenGetIntVal(token));
    case XY_TOK_FLOAT:
        return xyFloatValueCreate(paTokenGetIntVal(token));
    case XY_TOK_STRING:
        return xyStringValueCreate(paTokenGetText(token));
    case XY_TOK_IDENT:
        sym = utSymCreate((char *)paTokenGetText(token));
        return xySymValueCreate(sym);
    case XY_TOK_NEWLINE:
        return xySymValueCreate(paNEWLINESym);
    case XY_TOK_EOF:
        return xySymValueCreate(paEOFSym);
    // TODO: Do we need any of these
    case XY_TOK_CHAR:
    case XY_TOK_OPERATOR:
    case XY_TOK_BOOL:
    default:
        utExit("Unable to convert token to value");
    }
    return xyValueNull; // Dummy return;
}

// Push a state onto the stack.
static inline xyState push(xyStateArray states, xyValueArray values, xyState state, xyValue value) {
    xyStateArrayAppendState(states, state);
    xyValueArrayAppendValue(values, value);
    return state;
}

// Pop statesToPop states off the stack.
static void pop(xyStateArray states, xyValueArray values, uint32 statesToPop) {
    xyStateArraySetUsedState(states, xyStateArrayGetUsedState(states) - statesToPop);
    uint32 start = xyValueArrayGetUsedValue(values) - statesToPop;
    for(uint32 i = 0; i < statesToPop; i++) {
        xyValue value = xyValueArrayGetiValue(values, start + i);
        if(value != xyValueNull) {
            xyValueDestroy(value);
        }
    }
    xyValueArraySetUsedValue(values, xyValueArrayGetUsedValue(values) - statesToPop);
}

// Return the top state on the stack.
static inline xyState top(xyStateArray states) {
    return xyStateArrayGetiState(states, xyStateArrayGetUsedState(states) - 1);
}

// Forward declaration for recursion.
static xyValue executeMap(xyMap map, xyValueArray values, uint32 statesToPop, paToken token);

// Execute a concatenation map.
static xyValue executeConcatMap(xyMap map, xyValueArray values, uint32 statesToPop, paToken token) {
    xyMap leftMap = xyMapGetFirstMap(map);
    xyMap rightMap = xyMapGetNextMapMap(leftMap);
    xyValue leftValue = executeMap(leftMap, values, statesToPop, token);
    xyValue rightValue = executeMap(rightMap, values, statesToPop, token);
    if(xyValueGetType(leftValue) != XY_LIST) {
        paError(token, "Expected list value in concat map");
    }
    xyListAppendValue(xyValueGetListVal(leftValue), rightValue);
    return leftValue;
}

// Execute a list map.
static xyValue executeListMap(xyMap map, xyValueArray values, uint32 statesToPop, paToken token) {
    xyList list = xyListAlloc();
    xyMap child;
    xyForeachMapMap(map, child) {
        xyValue value = executeMap(child, values, statesToPop, token);
        xyListAppendValue(list, value);
    } xyEndMapMap;
    return xyListValueCreate(list);
}

// Execute a value map.
static xyValue executeValueMap(xyMap map, xyValueArray values, uint32 statesToPop, paToken token) {
    uint32 start = xyValueArrayGetUsedValue(values) - statesToPop;
    xyValue value = xyValueArrayGetiValue(values,  start + xyMapGetPosition(map));
    xyValueArraySetiValue(values, start + xyMapGetPosition(map), xyValueNull);
    return value;
}

// Execute a default map, by just putting all the values in a list.
static xyValue executeDefaultMap(xyValueArray values, uint32 statesToPop, paToken token) {
    uint32 start = xyValueArrayGetUsedValue(values) - statesToPop;
    if(statesToPop == 1) {
        xyValue value = xyValueArrayGetiValue(values, start);
        xyValueArraySetiValue(values, start, xyValueNull);
        return value;
    }
    xyList list = xyListAlloc();
    for(uint32 i = 0; i < statesToPop; i++) {
        xyValue value = xyValueArrayGetiValue(values, start + i);
        if(value == xyValueNull) {
            paError(token, "Used value twice in map");
        }
        xyValueArraySetiValue(values, start + i, xyValueNull);
        xyListAppendValue(list, value);
    }
    return xyListValueCreate(list);
}

// Execute a map expression to combinethe values on the top of the stack.
static xyValue executeMap(xyMap map, xyValueArray values, uint32 statesToPop, paToken token) {
    if(map == xyMapNull) {
        return executeDefaultMap(values, statesToPop, token);
    }
    switch(xyMapGetType(map)) {
    case XY_MAP_CONCAT:
        return executeConcatMap(map, values, statesToPop, token);
    case XY_MAP_LIST:
        return executeListMap(map, values, statesToPop, token);
    case XY_MAP_VALUE:
        return executeValueMap(map, values, statesToPop, token);
    default:
        utExit("Unknown map type");
    }
    return xyValueNull; // Dummy return
}

// Parse input tokens util we accept, or find an error.
static xyValue parseUntilAccept(xyParser parser, xyStateArray states, xyValueArray values) {
    xyState state = top(states);
    paToken token = paLex();
    xyValue value = buildTokenValue(token);
    while(true) {
        paPrintToken(token);
        printStack(states, values);
        xyMtoken mtoken = paTokenGetMtoken(token);
        xyAction action = xyStateFindAction(state, mtoken);
        if(action == xyActionNull) {
            paError(token, "Syntax error");
        }
        xyMtoken reduceMtoken;
        xyValue reducedValue;
        xyAction gotoAction;
        uint32 statesToPop;
        switch(xyActionGetType(action)) {
        case XY_ACCEPT:
            return xyValueArrayGetiValue(values, 1);
        case XY_SHIFT:
            state = push(states, values, xyActionGetDestState(action), value);
            token = paLex();
            value = buildTokenValue(token);
            break;
        case XY_REDUCE:
            reduceMtoken = xyActionGetReduceMtoken(action);
            printf("Reducing %s\n", xyMtokenGetName(reduceMtoken));
            statesToPop = xyActionGetStatesToPop(action);
            reducedValue = executeMap(xyActionGetMap(action), values, statesToPop, token);
            pop(states, values, statesToPop);
            state = top(states);
            gotoAction = xyStateFindAction(state, reduceMtoken);
            state = xyActionGetDestState(gotoAction);
            push(states, values, state, reducedValue);
            break;
        default:
            utExit("Unknown action type");
        }
    }
}

// Parse the input file using the action-goto table.
xyValue paParse(FILE *file, xyParser parser) {
    paFile = file;
    paDatabaseStart();
    utf8Start();
    paNEWLINESym = utSymCreate("NEWLINE");
    paEOFSym = utSymCreate("EOF");
    paLexerStart(parser);
    xyStateArray states = xyStateArrayAlloc();
    xyValueArray values = xyValueArrayAlloc();
    push(states, values, xyParserGetiState(parser, 0), xyValueNull);
    xyValue value = parseUntilAccept(parser, states, values);
    xyValueArrayFree(values);
    xyStateArrayFree(states);
    paLexerStop();
    utf8Stop();
    paDatabaseStop();
    return value;
}
