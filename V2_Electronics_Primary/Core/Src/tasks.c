#include "tasks.h"

#include "sensors.h"
#include "abort.h"
#include "command.h"
#include "valves.h"
#include "telemetry.h"

void Task_Fast(void)
{
    /*
     * Highest-priority foreground task.
     * Immediate redline was already checked in ADC DMA callback.
     * Here we enforce the latched fault quickly.
     */
    if (Sensors_IsRedlineLatched())
    {
        Abort_Latch(ABORT_REASON_REDLINE);
        //Valves_ForceSafeState();
        return;
    }

    /*
     * If a new sensor frame exists, do non-ISR processing now.
     */
    if (Sensors_TakeFrameReadyFlag())
    {
        Sensors_UpdateEngineeringValues();
    }

    /*
     * If abort got latched by some other path, still force safe outputs.
     */
    if (Abort_IsLatched())
    {
        //Valves_ForceSafeState();
    }
}

void Task_Mid(void)
{
    /*
     * Commands/state machine belong here, not in interrupt context.
     */
    Commands_Process();

    if (Abort_IsLatched())
    {
        //Valves_ForceSafeState();
        return;
    }

    //Valves_UpdateStateMachine();
}

void Task_Slow(void)
{
    /*
     * Slow housekeeping only.
     * Keep it non-blocking if possible.
     */
    //Telemetry_SendHousekeeping();
}
