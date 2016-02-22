#include "parse_int.h"

FILE *paFile;
uint32 paLineNum;

// Push a state onto the stack.
static inline void push(xyStateArray stack, xyState state) {
    xyStateArrayAppendState(stack, state);
}

// Pop statesToPop states off the stack.
static void pop(xyStateArray stack, uint32 statesToPop) {
    xyStateArraySetUsedState(stack, xyStateArrayGetUsedState(stack) - statesToPop);
}

// Return the top state on the stack.
static inline xyState top(xyStateArray stack) {
    return xyStateArrayGetiState(stack, xyStateArrayGetUsedState(stack) - 1);
}

// Parse input tokens util we accept, or find an error.
static void parseUntilAccept(xyParser parser, xyStateArray stack) {
    while(true) {
        xyState state = top(stack);
        paToken token = paLex();
        xyMtoken mtoken = paTokenGetMtoken(token);
        xyAction action = xyStateFindAction(state, mtoken);
        xyMtoken reduceMtoken;
        xyAction gotoAction;
        switch(xyActionGetType(action)) {
        case XY_ACCEPT:
            return;
        case XY_SHIFT:
            push(stack, xyActionGetDestState(action));
            break;
        case XY_REDUCE:
            reduceMtoken = xyActionGetReduceMtoken(action);
            printf("Reducing %s\n", xyMtokenGetName(reduceMtoken));
            pop(stack, xyActionGetStatesToPop(action));
            gotoAction = xyStateFindAction(top(stack), reduceMtoken);
            push(stack, xyActionGetDestState(gotoAction));
            break;
        default:
            utExit("Unknown action type");
        }
    }
}

// Parse the input file using the action-goto table.
bool paParse(FILE *file, xyParser parser) {
    paFile = file;
    paDatabaseStart();
    paLexerStart(parser);
    xyStateArray stack = xyStateArrayAlloc();
    push(stack, xyParserGetiState(parser, 0));
    parseUntilAccept(parser, stack);
    xyStateArrayFree(stack);
    paLexerStop();
    paDatabaseStop();
    return true;
}
