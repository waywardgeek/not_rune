#include "database.h"

// Print the state.
void xyPrintState(xyState state) {
    printf("state %u", xyStateGetAGTableIndex(state));
    xyAction action;
    xyForeachStateAction(state, action) {
        printf(" %s:", xyMtokenGetName(xyActionGetMtoken(action)));
        switch(xyActionGetType(action)) {
        case XY_GOTO:
            printf("g%u", xyStateGetAGTableIndex(xyActionGetDestState(action)));
            break;
        case XY_SHIFT:
            printf("s%u", xyStateGetAGTableIndex(xyActionGetDestState(action)));
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
    putchar('\n');
}

// Print out the xyAGTable.
void xyPrintAGTable(xyAGTable agtable) {
    printf("ActionGoto Table\n");
    xyState state;
    xyForeachAGTableState(agtable, state) {
        xyPrintState(state);
    } xyEndAGTableState;
}
