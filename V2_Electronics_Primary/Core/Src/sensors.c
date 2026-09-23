#include "sensors.h"

#include "stm32f4xx_hal.h"
#include <string.h>

extern ADC_HandleTypeDef hadc1;

/*
 * Raw DMA destination buffer.
 * Keep this as uint16_t for normal 12-bit ADC results.
 */
static uint16_t adcDmaBuffer[SENSOR_ADC_CHANNEL_COUNT];

/*
 * Latest complete sensor frame copied out of DMA buffer.
 */
static volatile SensorSnapshot latestSnapshot;

/*
 * Flags shared between ISR and tasks.
 */
static volatile bool sensorFrameReady = false;
static volatile bool redlineLatched   = false;

/*
 * --------------------------------------------------------------------------
 * Raw redline thresholds
 * --------------------------------------------------------------------------
 * These are placeholder ADC-count thresholds.
 * You must replace them with real calibrated values.
 *
 * Example:
 *   12-bit ADC full-scale = 4095
 *   If redline voltage = 2.50V on 3.3V ref:
 *   threshold_count = 2.50 / 3.3 * 4095 ≈ 3102
 */
#define PT_Channel1_RAW_REDLINE_HIGH   3100u


/*
 * Optional low thresholds if needed.
 * Set to 0 or remove if not used.
 */
#define PT_Channel1_REDLINE_LOW    50u


static bool Sensors_CheckImmediateRawRedline(const uint16_t *raw){
    if ((raw[ADC_IDX_PT_Channel1] > PT_Channel1_RAW_REDLINE_HIGH) || (raw[ADC_IDX_PT_Channel1] < PT_Channel1_REDLINE_LOW)){
        return true;
    }
}

static float ConvertPressureCountsToEngineering(uint16_t counts){
    /*
     * Placeholder conversion.
     * Replace with real sensor scaling and calibration.
     */
    return (float)counts;
}

static float ConvertLoadCountsToEngineering(uint16_t counts){
    /*
     * Placeholder conversion.
     * Replace with real bridge amplifier/load calibration.
     */
    return (float)counts;
}

void Sensors_Init(void){
    sensorFrameReady = false;
    redlineLatched   = false;

    memset((void *)&latestSnapshot, 0, sizeof(latestSnapshot));
    memset((void *)adcDmaBuffer, 0, sizeof(adcDmaBuffer));

    /*
     * Start continuous ADC DMA scan.
     * CubeMX should configure ADC + DMA circular mode appropriately.
     */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcDmaBuffer, SENSOR_ADC_CHANNEL_COUNT);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
    if (hadc->Instance == ADC1){
        /*
         * Copy raw data out of DMA buffer first.
         * Keep ISR short.
         */
        for (uint32_t i = 0; i < SENSOR_ADC_CHANNEL_COUNT; i++){
            latestSnapshot.adcRaw[i] = adcDmaBuffer[i];
        }

        /*
         * Immediate raw redline check in ISR context.
         * This is Pattern B.
         */
        if (Sensors_CheckImmediateRawRedline((const uint16_t *)latestSnapshot.adcRaw)){
            redlineLatched = true;
        }

        /*
         * Notify foreground task that a fresh frame exists.
         */
        sensorFrameReady = true;
    }
}

void Sensors_GetLatestSnapshot(SensorSnapshot *snapshot){
    __disable_irq();
    memcpy(snapshot, (const void *)&latestSnapshot, sizeof(SensorSnapshot));
    __enable_irq();
}

bool Sensors_TakeFrameReadyFlag(void){
    bool wasReady;
    __disable_irq();
    wasReady = sensorFrameReady;
    sensorFrameReady = false;
    __enable_irq();

    return wasReady;
}

bool Sensors_IsRedlineLatched(void){
    return redlineLatched;
}

void Sensors_ClearRedlineLatch(void){
    redlineLatched = false;
}

void Sensors_UpdateEngineeringValues(void){
    /*
     * Convert in foreground, not ISR.
     * We update the shared snapshot directly.
     * If you want stricter atomicity, copy raw data to a local temp first.
     */
    latestSnapshot.PT_Channel1_Read  = ConvertPressureCountsToEngineering(latestSnapshot.adcRaw[ADC_IDX_PT_Channel1]);
    latestSnapshot.PT_Channel2_Read  = ConvertPressureCountsToEngineering(latestSnapshot.adcRaw[ADC_IDX_PT_Channel2]);
    latestSnapshot.PT_Channel3_Read  = ConvertPressureCountsToEngineering(latestSnapshot.adcRaw[ADC_IDX_PT_Channel3]);
    latestSnapshot.PT_Channel4_Read  = ConvertPressureCountsToEngineering(latestSnapshot.adcRaw[ADC_IDX_PT_Channel4]);
    latestSnapshot.PT_Channel5_Read  = ConvertPressureCountsToEngineering(latestSnapshot.adcRaw[ADC_IDX_PT_Channel5]);
    latestSnapshot.PT_Channel6_Read  = ConvertPressureCountsToEngineering(latestSnapshot.adcRaw[ADC_IDX_PT_Channel6]);

    latestSnapshot.LC1_A_Read = ConvertLoadCountsToEngineering(latestSnapshot.adcRaw[ADC_IDX_LC1_A]);
    latestSnapshot.LC1_B_Read = ConvertLoadCountsToEngineering(latestSnapshot.adcRaw[ADC_IDX_LC1_B]);
    latestSnapshot.LC2_A_Read = ConvertLoadCountsToEngineering(latestSnapshot.adcRaw[ADC_IDX_LC2_A]);
    latestSnapshot.LC2_B_Read = ConvertLoadCountsToEngineering(latestSnapshot.adcRaw[ADC_IDX_LC2_B]);
    latestSnapshot.LC3_A_Read = ConvertLoadCountsToEngineering(latestSnapshot.adcRaw[ADC_IDX_LC3_A]);
    latestSnapshot.LC3_B_Read = ConvertLoadCountsToEngineering(latestSnapshot.adcRaw[ADC_IDX_LC3_B]);

}
