#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include <stdbool.h>

#define SENSOR_ADC_CHANNEL_COUNT  12u

typedef enum
{
    ADC_IDX_PT_Channel1 = 0,
    ADC_IDX_PT_Channel2,
    ADC_IDX_PT_Channel3,
    ADC_IDX_PT_Channel4,
    ADC_IDX_PT_Channel5,
    ADC_IDX_PT_Channel6,
    ADC_IDX_LC1_A,
    ADC_IDX_LC1_B,
    ADC_IDX_LC2_A,
    ADC_IDX_LC2_B,
    ADC_IDX_LC3_A,
    ADC_IDX_LC3_B
} SensorAdcIndex;

typedef struct
{
    uint16_t adcRaw[SENSOR_ADC_CHANNEL_COUNT];

    float PT_Channel1_Read;
    float PT_Channel2_Read;
    float PT_Channel3_Read;
    float PT_Channel4_Read;
    float PT_Channel5_Read;
    float PT_Channel6_Read;
    float LC1_A_Read;
    float LC1_B_Read;
    float LC2_A_Read;
    float LC2_B_Read;
    float LC3_A_Read;
    float LC3_B_Read;

} SensorSnapshot;


void Sensors_Init(void);

/*
 * Copies the latest snapshot into the caller's struct.
 */
void Sensors_GetLatestSnapshot(SensorSnapshot *snapshot);

/*
 * True if a DMA frame has arrived since the last consume.
 * This is optional but useful for the fast task.
 */
bool Sensors_TakeFrameReadyFlag(void);

/*
 * True if an immediate ISR-level redline was detected.
 */
bool Sensors_IsRedlineLatched(void);

/*
 * Optional: clear only if your system design allows it.
 * Many systems keep aborts latched until reset.
 */
void Sensors_ClearRedlineLatch(void);

/*
 * Convert the latest raw ADC values into engineering values.
 * Call from Task_Fast(), not from ISR.
 */
void Sensors_UpdateEngineeringValues(void);

#endif
