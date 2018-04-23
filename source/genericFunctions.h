/*
 * genericFunctions.h
 *
 *  Created on: Apr 19, 2018
 *      Author: Andres Hernandez
 */

#ifndef GENERICFUNCTIONS_H_
#define GENERICFUNCTIONS_H_

#include "stdint.h"

void dacInit();

void dacSetValue(uint8_t value);

void partBlock(uint16_t *dataHigh, uint16_t *dataLow, uint32_t data);

void pitInit();

void pitSetPeriod(uint32_t period);

void pitStartTimer();

uint8_t getIsrFlag();

void setIsrFlag(uint8_t flag);

#endif /* GENERICFUNCTIONS_H_ */
