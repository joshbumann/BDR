#ifndef VALVES_H
#define VALVES_H
#include "main.h"

typedef enum {
	FUEL_N2_VLV = 0,
	LOX_N2_VLV,
	MFV_VLV,
	MOV_VLV,
	FUEL_VENT_VLV,
	LOX_VENT_VLV,
	FUEL_PURGE_VLV,
	LOX_PURGE_VLV,
} ValveEnum;

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} GpioMap;

void openValve(ValveEnum Valve);
void closeValve(ValveEnum Valve);
void closeAllValves();
void toggleValve(ValveEnum Valve);

#endif
