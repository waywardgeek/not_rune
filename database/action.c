#include "database.h"

// Report a shift/reduce or reduce/reduce conflict.
static void reportConflict(xyActionType type1, xyAction action) {
    xyActionType type2 = xyActionGetType(action);
    xyMtoken mtoken = xyActionGetMtoken(action);
    xyState state = xyActionGetState(action);
    if(type1 == XY_SHIFT || type2 == XY_SHIFT) {
        utWarning("Shift/Reduce conflict in state %u on '%s'", xyStateGetParserIndex(state),
            xyMtokenGetName(mtoken));
    } else {
        utWarning("Reduce/Reduce conflict in state %u on '%s'", xyStateGetParserIndex(state),
            xyMtokenGetName(mtoken));
    }
}

// Build a new action.  If an action for the mtoken already exists, report the
// conflict, and use the original one.
static xyAction createAction(xyActionType type, xyState state, xyMtoken mtoken) {
    xyAction action = xyStateFindAction(state, mtoken);
    if(action != xyActionNull) {
        reportConflict(type, action);
        return xyActionNull;
    }
    action = xyActionAlloc();
    xyActionSetType(action, type);
    xyActionSetMtoken(action, mtoken);
    xyStateInsertAction(state, action);
    return action;
}

// Create a GOTO action.
xyAction xyGotoActionCreate(xyState state, xyMtoken mtoken, xyState destState) {
    xyAction action = createAction(XY_GOTO, state, mtoken);
    if(action == xyActionNull) {
        return xyActionNull;
    }
    xyActionSetDestState(action, destState);
    return action;
}

// Create a SHIFT action.
xyAction xyShiftActionCreate(xyState state, xyMtoken mtoken, xyState destState) {
    xyAction action = createAction(XY_SHIFT, state, mtoken);
    if(action == xyActionNull) {
        return xyActionNull;
    }
    xyActionSetDestState(action, destState);
    return action;
}

// Create a REDUCE action.
xyAction xyReduceActionCreate(xyState state, xyMtoken mtoken, xyMtoken reduceMtoken, uint32 statesToPop) {
    xyAction action = createAction(XY_REDUCE, state, mtoken);
    if(action == xyActionNull) {
        return xyActionNull;
    }
    xyActionSetReduceMtoken(action, reduceMtoken);
    xyActionSetStatesToPop(action, statesToPop);
    return action;
}

// Create an ACCEPT action.
xyAction xyAcceptActionCreate(xyState state, xyMtoken mtoken) {
    return createAction(XY_ACCEPT, state, mtoken);
}
