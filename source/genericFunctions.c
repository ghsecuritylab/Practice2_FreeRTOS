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
#define DACBUFFERBASE (0)
#define DACMAXVALUE	(0xFFF)
#define OFFSET		(2047)

uint8_t pitIsrFlag;

void dacInit()
{
    dac_config_t dacConfig;

	DAC_GetDefaultConfig(&dacConfig);
	DAC_Init(DAC0, &dacConfig);
	DAC_Enable(DAC0, true);
	DAC_SetBufferReadPointer(DAC0, DACBUFFERBASE);
}
void dacSetValue(int16_t dacValue)
{
    DAC_SetBufferValue(DAC0, DACBUFFERBASE, (dacValue + OFFSET));
}

void partBlock(int16_t *dataAudio, uint32_t *data, uint32_t len)
{
	uint16_t count;
	uint16_t countTmp = 0;
	int16_t part;
	uint32_t *direction;
	uint32_t dataFinal;
	int32_t temp1, temp2;
	uint32_t realLength;

	direction = (uint32_t*)*data;
	dataFinal = *direction;
	realLength = len / 2;

	for(count = 0; count < realLength; count++)
	{
		temp1 = dataFinal;
		temp2 = dataFinal;

		/*******************************************/
		temp1 &= (0xFFFF);
		part = temp1;
		part = part/16;

		dataAudio[countTmp] = part;
		/*******************************************/
		countTmp++;

		temp2 &= (0xFFFF0000);
		temp2 = temp2>>16;
		part = temp2;
		part = part/16;

		dataAudio[countTmp] = part;

		countTmp++;
		direction++;
		dataFinal = *direction;
	}
}

void pitInit()
{
	pit_config_t config_pit0;
	PIT_GetDefaultConfig(&config_pit0);

	PIT_Init (PIT, &config_pit0);
	PIT_EnableInterrupts (PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
	PIT_GetEnabledInterrupts(PIT, kPIT_Chnl_0);
}

void pitSetPeriod(void)
{
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_0,SECOND_TICK);
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


