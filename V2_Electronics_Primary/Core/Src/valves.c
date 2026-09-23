#include "main.h"
#include "valves.h"

static const GpioMap ValvePinPortMap[] = {
	[FUEL_N2_VLV] = {Fuel_N2_Vlv_GPIO_Port, Fuel_N2_Vlv_Pin},
	[LOX_N2_VLV] = {LOX_N2_Vlv_GPIO_Port, LOX_N2_Vlv_Pin},
	[MFV_VLV] = {MFV_Vlv_GPIO_Port, MFV_Vlv_Pin},
	[MOV_VLV] = {MOV_Vlv_GPIO_Port, MOV_Vlv_Pin},
	[FUEL_VENT_VLV] = {Fuel_Vent_Vlv_GPIO_Port, Fuel_Vent_Vlv_Pin},
	[LOX_VENT_VLV] = {LOX_Vent_Vlv_GPIO_Port, LOX_Vent_Vlv_Pin},
	[FUEL_PURGE_VLV] = {Fuel_Purge_Vlv_GPIO_Port, Fuel_Purge_Vlv_Pin},
	[LOX_PURGE_VLV] = {LOX_Purge_Vlv_GPIO_Port, LOX_Purge_Vlv_Pin},
};

void openValve(ValveEnum Valve){
	GPIO_TypeDef *GPIOx = ValvePinPortMap[Valve].port;
	uint16_t GPIO_Pin = ValvePinPortMap[Valve].pin;

	if(HAL_GPIO_ReadPin(GPIOx,GPIO_Pin) == GPIO_PIN_RESET){
		HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
	}
}

void closeValve(ValveEnum Valve){
	GPIO_TypeDef *GPIOx = ValvePinPortMap[Valve].port;
	uint16_t GPIO_Pin = ValvePinPortMap[Valve].pin;

	if(HAL_GPIO_ReadPin(GPIOx,GPIO_Pin) == GPIO_PIN_SET){
		HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	}
}

void closeAllValves(){
	closeValve(FUEL_N2_VLV);
	closeValve(LOX_N2_VLV);
	closeValve(MFV_VLV);
	closeValve(MOV_VLV);
	closeValve(FUEL_VENT_VLV);
	closeValve(LOX_VENT_VLV);
	closeValve(FUEL_PURGE_VLV);
	closeValve(LOX_PURGE_VLV);
}

void toggleValve(ValveEnum Valve){
	GPIO_TypeDef *GPIOx = ValvePinPortMap[Valve].port;
	uint16_t GPIO_Pin = ValvePinPortMap[Valve].pin;

	if(HAL_GPIO_ReadPin(GPIOx,GPIO_Pin) == GPIO_PIN_SET){
		HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	} else if (HAL_GPIO_ReadPin(GPIOx,GPIO_Pin) == GPIO_PIN_RESET){
		HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
	}
}
