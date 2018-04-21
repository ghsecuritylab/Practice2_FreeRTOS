/*
 * genericFunctions.c
 *
 *  Created on: Apr 19, 2018
 *      Author: Andres Hernandez
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include "genericFunctions.h"
#include "fsl_dac.h"
#include "fsl_pit.h"

#define DATA_LOW_OR 	0xFFFF
#define DATA_HIGH_OR	0xFFFF0000
#define BIT_SHIFTING	16


uint8_t pitIsrFlag;
#if 0
void PIT0_IRQHandler()
{
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
	PRINTF("\r\n Channel No.0 interrupt is occured !\n");

    pitIsrFlag = 1;

}
#endif
void partBlock(uint16_t *dataHigh, uint16_t *dataLow, uint32_t data)
{
	uint32_t lowPart;
	uint32_t highPart;

	lowPart = data;
	highPart = data;

	lowPart &= DATA_LOW_OR;
	highPart &= DATA_HIGH_OR;
	highPart = (highPart >> BIT_SHIFTING);

	*dataLow = (uint16_t)lowPart;
	*dataHigh = (uint16_t)highPart;
}

void pitInit()
{
	pit_config_t config_pit0;
	PIT_GetDefaultConfig(&config_pit0);

	PIT_Init (PIT, &config_pit0);
	PIT_EnableInterrupts (PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
	PIT_GetEnabledInterrupts(PIT, kPIT_Chnl_0);
}

void pitSetPeriod(uint32_t period)
{
	uint32_t time;
	time = 1/CLOCK_GetFreq(kCLOCK_BusClk);
	time *= period;
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_0,time);

}
void pitStartTimer()
{
    PIT_StartTimer(PIT, kPIT_Chnl_0);

}

void setIsrFlag(uint8_t flag)
{
	pitIsrFlag=flag;
}
uint8_t getIsrFlag()
{
	return pitIsrFlag;
}


