#ifndef ABORT_H
#define ABORT_H

#include <stdbool.h>

typedef enum
{
    ABORT_REASON_NONE = 0,
    ABORT_REASON_REDLINE,
    ABORT_REASON_COMMAND,
    ABORT_REASON_SENSOR_FAULT
} AbortReason;

void Abort_Latch(AbortReason reason);
bool Abort_IsLatched(void);

#endif
