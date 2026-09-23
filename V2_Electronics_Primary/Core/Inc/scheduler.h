#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

/*
 * "Due" counters.
 * These are incremented in the timer ISR and decremented in the main loop.
 *
 * Using counters instead of single-bit flags means that if the CPU is briefly
 * busy, you do not completely lose track of missed releases.
 */
extern volatile uint32_t fastTaskDue;
extern volatile uint32_t midTaskDue;
extern volatile uint32_t slowTaskDue;

uint8_t Scheduler_TakeTask(volatile uint32_t *counter);

/*
 * Initialize scheduler state.
 */
void Scheduler_Init(void);

/*
 * Main scheduler loop.
 * Never returns.
 */
void Scheduler_Run(void);

/*
 * Called from the 1 ms TIM2 interrupt callback.
 * Keep this very short.
 */
void Scheduler_Tick1msISR(void);

#endif
