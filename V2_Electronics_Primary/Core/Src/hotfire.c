#include "main.h"
#include "valves.h"

const uint8_t delay_closevent_openMBVs = 10; // HFS2 : the delay betewen closing the vents and opening the MBVs. Something small just to act as a buffer not to vent any unessesary N2
const uint8_t delay_MFV_MOV = 20;
const uint16_t delay_Bleed = 1000;
const uint8_t delay_closeMVs_openPurge = 0;       // Delay between closing the main valves and opening the the purge and vent lines
const uint16_t delay_purgeEnd = 5000;              // How long the purge is open to confirm shutdown
const uint8_t delay_closePurge_closeVents = 0;    // Short delay between closing the purge and vents so everything the the purge lines can escape

bool engineFired = 0;   // Helps with logic for ending hot fire. Changes if engine has been fired.
bool igniterFired = 0;
           // This is the delay between opening the main valves. We want the liquids to enter the injector at the same time, and this takes into account that the fuel needs to travel through the regen channels.
static uint8_t currentHFstate = 0;          // This variable is used to hold the hot fire states

void HF0toHF1(){
	// Tank press
	currentHFstate = 1;
	// Close vents, purge, and MPVs
	closeAllValves();

	// Small delay to ensure vent close
	//delay(delay_closevent_openMBVs);
	HAL_Delay(delay_closevent_openMBVs);

	// Open MBVs
	openValve(FUEL_N2_VLV);
	openValve(LOX_N2_VLV);
	// The MBVs being fully open is up to the user to determine
}
void handleHF1toHF2(){
	// Fire igniter
	currentHFstate = 2;
	//digitalWrite(igniterPin, HIGH);
}
void handleHF2toHF3(){
	currentHFstate = 3;
	// Open main prop valves, w/ delay for regen channels
	openValve(MFV_VLV);
	HAL_Delay(delay_MFV_MOV);
	openValve(MOV_VLV);

	// Write low to igniter pin
	//digitalWrite(igniterPin,LOW);
}
void handleHF3toHF4(){
	// End test
	currentHFstate = 4;

	// Open purge
	openValve(FUEL_PURGE_VLV);
	openValve(LOX_PURGE_VLV);

	// Close MPVs
	closeValve(MFV_VLV);
	closeValve(MOV_VLV);
}
void handleHardAbort(){
	// Hard abort: Close MBVs, open purge, and open vent simultaneously
	currentHFstate = 0;
	// Close all valves
	closeAllValves();

	// Open vent
	openValve(FUEL_VENT_VLV);
	openValve(LOX_VENT_VLV);

	// Open purge
	openValve(FUEL_PURGE_VLV);
	openValve(LOX_PURGE_VLV);
}

void handleHF2toHF1(){
	// Write low to igniter pin
	//digitalWrite(igniterPin,LOW);
}


void handleBleedLOXPress(){
	openValve(LOX_VENT_VLV);
	HAL_Delay(delay_Bleed);
	closeValve(LOX_VENT_VLV);
}
void handleSoftAbort(){
	// Change nothing, leave up to CF
}
