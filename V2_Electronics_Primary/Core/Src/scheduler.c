#include "scheduler.h"
#include "tasks.h"

#include "stm32f4xx_hal.h"

/*
 * Task periods in milliseconds.
 */
#define FAST_TASK_PERIOD_MS   1u
#define MID_TASK_PERIOD_MS    10u
#define SLOW_TASK_PERIOD_MS   100u

volatile uint32_t fastTaskDue = 0u;
volatile uint32_t midTaskDue  = 0u;
volatile uint32_t slowTaskDue = 0u;


/*
 * Atomically take one pending task release from a counter.
 */
uint8_t Scheduler_TakeTask(volatile uint32_t *counter){

	uint8_t shouldRun = 0u;
    __disable_irq();

    if (*counter > 0u){
        (*counter)--;
        shouldRun = 1u;
    }

    __enable_irq();
    return shouldRun;
}

void Scheduler_Init(void){
    fastTaskDue = 0u;
    midTaskDue  = 0u;
    slowTaskDue = 0u;
}

void Scheduler_Tick1msISR(void){
    static uint32_t fastDivider = 0u;
    static uint32_t midDivider  = 0u;
    static uint32_t slowDivider = 0u;

    fastDivider++;
    midDivider++;
    slowDivider++;

    if (fastDivider >= FAST_TASK_PERIOD_MS){
        fastDivider = 0u;
        fastTaskDue++;
    }

    if (midDivider >= MID_TASK_PERIOD_MS){
        midDivider = 0u;
        midTaskDue++;
    }

    if (slowDivider >= SLOW_TASK_PERIOD_MS){
        slowDivider = 0u;
        slowTaskDue++;
    }
}
