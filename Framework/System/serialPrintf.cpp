/*
 * serialPrintf.cpp
 *
 *  Created on: 21.03.2021
 *      Author: harald
 */

#include "serialPrintf.h"


int tx_printBuff(uint8_t* buffer, uint8_t len) {
	uint8_t outBuff[len+1];
	memcpy(outBuff, buffer, len);
	outBuff[len+1] = '\0';
	printf("%s", outBuff);
	return 0;
}

