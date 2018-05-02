/*
 * genericFunctions.h
 *
 *  Created on: Apr 19, 2018
 *      Author: Andres Hernandez
 */

#ifndef GENERICFUNCTIONS_H_
#define GENERICFUNCTIONS_H_

#include "stdint.h"

void dacInit(void);

void dacSetValue(int16_t value);

void partBlock(int16_t *dataAudio, uint32_t *data, uint32_t len);

void pitInit(void);

void pitSetPeriod(void);

void pitStartTimer(void);

uint8_t getIsrFlag(void);

void setIsrFlag(uint8_t flag);

#endif /* GENERICFUNCTIONS_H_ */
