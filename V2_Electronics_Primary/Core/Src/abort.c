#include "abort.h"

static volatile bool abortLatched = false;
static volatile AbortReason abortReason = ABORT_REASON_NONE;

void Abort_Latch(AbortReason reason)
{
    abortLatched = true;

    if (abortReason == ABORT_REASON_NONE)
    {
        abortReason = reason;
    }
}

bool Abort_IsLatched(void)
{
    return abortLatched;
}
