#ifndef COMMANDS_H
#define COMMANDS_H

#include "main.h"
#include <stddef.h>

typedef enum {
  CMD_FUEL_N2 = 'A',
  CMD_LOX_N2 = 'B',
  CMD_MAIN_FUEL = 'C',
  CMD_MAIN_OX = 'D',
  CMD_FUEL_VENT = 'E',
  CMD_LOX_VENT = 'F',
  CMD_FUEL_PURGE = 'G',
  CMD_LOX_PURGE = 'H',
  CMD_TOGGLE_MAIN = 'I',
  CMD_TOGGLE_N2 = 'J',
  CMD_TOGGLE_VENTS = 'K',
  CMD_TOGGLE_PURGE = 'L',
  CMD_CLOSE_ALL = 'M',
  CMD_HARD_ABORT = '0',
  CMD_ENTER_HOTFIRE = '1',
  CMD_ENTER_COLDFLOW = 'U',
  CMD_FIRE_IGNITER = '2',
  CMD_OPEN_MAIN_PROPELLANTS = '3',
  CMD_END_TEST = '4',
  CMD_BLEED_LOX = 'Z',
  CMD_SOFT_ABORT = 'S'
} CommandCode;

typedef void (*CommandHandler)(void);

typedef struct {
    CommandCode cmd;
    CommandHandler handler;
    const char *name;
} CommandMapping;

extern const CommandMapping ColdFlowCommandMap[];
extern const CommandMapping HotFireCommandMap[];

void Command_Init();
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
bool Commands_PopByte(uint8_t *byte);
bool Commands_HasByte(void);
const char *Command_GetName(CommandCode cmd);
void Commands_HandleByte();
void Commands_Process();
void bleedLoxPress();
void softAbort();

#endif
