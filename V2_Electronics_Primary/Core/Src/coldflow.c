#include "main.h"
#include "command.h"
#include "coldflow.h"
#include "hotfire.h"
#include "valves.h"

extern uint8_t currentState;

void handleEnterColdFlowCommand(void){
	currentState = ColdFlowState;
}

void handleFuelN2Command(void) {
    toggleValve(FUEL_N2_VLV);
}

void handleLoxN2Command(void) {
    toggleValve(LOX_N2_VLV);
}

void handleMainFuelCommand(void) {
    toggleValve(MFV_VLV);
}

void handleMainOxCommand(void) {
    toggleValve(MOV_VLV);
}

void handleFuelVentCommand(void) {
    toggleValve(FUEL_VENT_VLV);
}

void handleLoxVentCommand(void) {
    toggleValve(LOX_VENT_VLV);
}

void handleFuelPurgeCommand(void) {
    toggleValve(FUEL_PURGE_VLV);
}

void handleLoxPurgeCommand(void) {
    toggleValve(LOX_PURGE_VLV);
}

void handleToggleMainCommand(void) {
    toggleValve(MFV_VLV);
    toggleValve(MOV_VLV);
}

void handleToggleN2Command(void) {
    toggleValve(FUEL_N2_VLV);
    toggleValve(LOX_N2_VLV);
}

void handleToggleVentsCommand(void) {
    toggleValve(FUEL_VENT_VLV);
    toggleValve(LOX_VENT_VLV);
}

void handleTogglePurgeCommand(void) {
    toggleValve(FUEL_PURGE_VLV);
    toggleValve(LOX_PURGE_VLV);
}

void handleCloseAllCommand(void) {
    closeAllValves();
}

void handleEnterHotfireCommand(void){
	currentState = HotFireState;
	HF0toHF1();
}

