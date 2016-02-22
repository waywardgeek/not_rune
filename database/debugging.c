#include "database.h"

// Print the state.
void xyPrintState(xyState state) {
    printf("state %u", xyStateGetParserIndex(state));
    xyAction action;
    xyForeachStateAction(state, action) {
        printf(" %s:", xyMtokenGetName(xyActionGetMtoken(action)));
        switch(xyActionGetType(action)) {
        case XY_GOTO:
            printf("g%u", xyStateGetParserIndex(xyActionGetDestState(action)));
            break;
        case XY_SHIFT:
            printf("s%u", xyStateGetParserIndex(xyActionGetDestState(action)));
            break;
        case XY_REDUCE:
            printf("r%u/%s", xyActionGetStatesToPop(action),
                xyMtokenGetName(xyActionGetReduceMtoken(action)));
            break;
        case XY_ACCEPT:
            printf("ACCEPT");
            break;
        default:
            utExit("Unknown action type");
        }
    } xyEndStateAction;
    if(xyStateIgnoreNewlines(state)) {
        printf(" ignore_newlines");
    }
    putchar('\n');
}

// Print out the xyParser.
void xyPrintParser(xyParser parser) {
    printf("Parser Action/GOTO Table\n");
    xyState state;
    xyForeachParserState(parser, state) {
        xyPrintState(state);
    } xyEndParserState;
}
