#include "command.h"
#include "main.h"
#include "valves.h"
#include "stm32f4xx_hal.h"
#include "hotfire.h"
#include "coldflow.h"
#include <string.h>

extern UART_HandleTypeDef huart2;

#define CMD_RING_BUFFER_SIZE 32u

const CommandMapping IdleCommandMap[] = {
    {CMD_ENTER_HOTFIRE,  handleEnterHotfireCommand,  "Entering Hotfire Mode\r\n"},
    {CMD_ENTER_COLDFLOW, handleEnterColdFlowCommand, "Entering ColdFlow Mode\r\n"},
};

const CommandMapping ColdFlowCommandMap[] = {
    {CMD_FUEL_N2,       handleFuelN2Command,      "Toggle Fuel N2 Valve\r\n"},
    {CMD_LOX_N2,        handleLoxN2Command,       "Toggle LOX N2 Valve\r\n"},
    {CMD_MAIN_FUEL,     handleMainFuelCommand,    "Toggle Main Fuel Valve\r\n"},
    {CMD_MAIN_OX,       handleMainOxCommand,      "Toggle Main OX Valve\r\n"},
    {CMD_FUEL_VENT,     handleFuelVentCommand,    "Toggle Fuel Vent Valve\r\n"},
    {CMD_LOX_VENT,      handleLoxVentCommand,     "Toggle LOX Vent Valve\r\n"},
    {CMD_FUEL_PURGE,    handleFuelPurgeCommand,   "Toggle Fuel Purge Valve\r\n"},
    {CMD_LOX_PURGE,     handleLoxPurgeCommand,    "Toggle LOX Purge Valve\r\n"},
    {CMD_TOGGLE_MAIN,   handleToggleMainCommand,  "Toggle Main Valves\r\n"},
    {CMD_TOGGLE_N2,     handleToggleN2Command,    "Toggle N2 Valves\r\n"},
    {CMD_TOGGLE_VENTS,  handleToggleVentsCommand, "Toggle Vent Valves\r\n"},
    {CMD_TOGGLE_PURGE,  handleTogglePurgeCommand, "Toggle Purge Valves\r\n"},
    {CMD_CLOSE_ALL,     handleCloseAllCommand,    "Close All Valves\r\n"},
    {CMD_ENTER_HOTFIRE, handleEnterHotfireCommand,"Entering Hotfire Mode\r\n"},
};

const CommandMapping HotFireCommandMap[] = {
    {CMD_FIRE_IGNITER, handleHF1toHF2,         "Initiate Ignition Sequence\r\n"},
    {CMD_END_TEST,     handleHF3toHF4,         "Ending Test\r\n"},
    {CMD_SOFT_ABORT,   handleSoftAbort,        "Initiate SOFT ABORT\r\n"},
    {CMD_HARD_ABORT,   handleHardAbort,        "Initiate HARD ABORT\r\n"},
    {CMD_BLEED_LOX,    handleBleedLOXPress,    "Bleed LOX Pressure\r\n"},
};

static uint8_t rxByte;

static volatile uint8_t cmdRingBuf[CMD_RING_BUFFER_SIZE];
static volatile uint16_t cmdHead = 0u;
static volatile uint16_t cmdTail = 0u;
static volatile uint32_t cmdOverflowCount = 0u;

static bool RingBuffer_PushByte(uint8_t byte)
{
    uint16_t nextHead = (uint16_t)((cmdHead + 1u) % CMD_RING_BUFFER_SIZE);

    if (nextHead == cmdTail) {
        cmdOverflowCount++;
        return false;
    }

    cmdRingBuf[cmdHead] = byte;
    cmdHead = nextHead;
    return true;
}

void Command_Init(void)
{
    HAL_GPIO_WritePin(RX_EN_GPIO_Port, RX_EN_Pin, GPIO_PIN_RESET); // RX mode

    cmdHead = 0u;
    cmdTail = 0u;
    cmdOverflowCount = 0u;

    HAL_UART_Receive_IT(&huart2, &rxByte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        HAL_UART_Transmit(&huart2, &rxByte, 1, 100);
        (void)RingBuffer_PushByte(rxByte);
        HAL_UART_Receive_IT(&huart2, &rxByte, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        // Restart IT receive on error
        HAL_UART_Receive_IT(&huart2, &rxByte, 1);
    }
}

bool Commands_PopByte(uint8_t *byte)
{
    bool ok = false;

    if (byte == NULL) {
        return false;
    }

    __disable_irq();

    if (cmdHead != cmdTail) {
        *byte = cmdRingBuf[cmdTail];
        cmdTail = (uint16_t)((cmdTail + 1u) % CMD_RING_BUFFER_SIZE);
        ok = true;
    }

    __enable_irq();
    return ok;
}

bool Commands_HasByte(void)
{
    return (cmdHead != cmdTail);
}

static void Command_SendString(const char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 100);
}

void Commands_Process(void)
{
    uint8_t byte;

    while (Commands_PopByte(&byte)) {
        CommandCode cmd = (CommandCode)byte;

        if (currentState == Idle) {
            for (uint32_t i = 0; i < (sizeof(IdleCommandMap) / sizeof(IdleCommandMap[0])); i++) {
                if (IdleCommandMap[i].cmd == cmd) {
                    IdleCommandMap[i].handler();
                    Command_SendString(IdleCommandMap[i].name);
                    break;
                }
            }
        }
        else if (currentState == ColdFlowState) {
            for (uint32_t i = 0; i < (sizeof(ColdFlowCommandMap) / sizeof(ColdFlowCommandMap[0])); i++) {
                if (ColdFlowCommandMap[i].cmd == cmd) {
                    ColdFlowCommandMap[i].handler();
                    Command_SendString(ColdFlowCommandMap[i].name);
                    break;
                }
            }
        }
        else if (currentState == HotFireState) {
            for (uint32_t i = 0; i < (sizeof(HotFireCommandMap) / sizeof(HotFireCommandMap[0])); i++) {
                if (HotFireCommandMap[i].cmd == cmd) {
                    HotFireCommandMap[i].handler();
                    Command_SendString(HotFireCommandMap[i].name);
                    break;
                }
            }
        }
    }
}
