#include <string.h>

#include "main.h"
#include "sensors.h"

extern UART_HandleTypeDef huart2;

float PT_Channel1_Read, PT_Channel2_Read, PT_Channel3_Read, PT_Channel4_Read, PT_Channel5_Read, PT_Channel6_Read,
		 LC1_A_Read,LC1_B_Read,LC2_A_Read,LC2_B_Read,LC3_A_Read,LC3_B_Read;

void Telemetry_Init(void){
	HAL_GPIO_WritePin(TX_EN_GPIO_Port, TX_EN_Pin, GPIO_PIN_SET); // Pull DE High to enable TX operation
}

void Telemetry_SendHousekeeping(void){
	char dataString[256];
	SensorSnapshot snapshot;
	// Ensure that Read Data is Converted
	Sensors_UpdateEngineeringValues();
	Sensors_GetLatestSnapshot(&snapshot);


	PT_Channel1_Read = snapshot.PT_Channel1_Read;
	PT_Channel2_Read = snapshot.PT_Channel2_Read;
	PT_Channel3_Read = snapshot.PT_Channel3_Read;
	PT_Channel4_Read = snapshot.PT_Channel4_Read;
	PT_Channel5_Read = snapshot.PT_Channel5_Read;
	PT_Channel6_Read = snapshot.PT_Channel6_Read;
	LC1_A_Read = snapshot.LC1_A_Read;
	LC1_B_Read = snapshot.LC1_B_Read;
	LC2_A_Read = snapshot.LC2_A_Read;
	LC2_B_Read = snapshot.LC2_B_Read;
	LC3_A_Read = snapshot.LC3_A_Read;
	LC3_B_Read = snapshot.LC3_B_Read;

	int len = snprintf(dataString, sizeof(dataString),
	         "<%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f>\r\n",
			 PT_Channel1_Read,
			 PT_Channel2_Read,
			 PT_Channel3_Read,
			 PT_Channel4_Read,
			 PT_Channel5_Read,
			 PT_Channel6_Read,
			 LC1_A_Read,
			 LC1_B_Read,
			 LC2_A_Read,
			 LC2_B_Read,
			 LC3_A_Read,
			 LC3_B_Read);

	if (len > 0 && len < sizeof(dataString))
	{
	    HAL_UART_Transmit(&huart2, (uint8_t *)dataString, len, 100);
	}
	HAL_UART_Transmit(&huart2, (uint8_t *)currentState, 1, 100);
}
