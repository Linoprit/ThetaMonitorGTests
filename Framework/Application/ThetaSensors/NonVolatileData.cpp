/*
 * NonVolatileData.cpp
 *
 *  Created on: 15.04.2021
 *      Author: harald
 */

#include <Application/ThetaSensors/NonVolatileData.h>
#include <Devices/Eeprom_at24c256/AT24Cxxx.h>
#include <Sockets/CrcSocket.h>
#include <Libraries/HelpersLib.h>
#include <System/serialPrintf.h>
#include <string>

namespace NonVolDat {
uint16_t NonVolatileData::_currAddress = 0;

NonVolatileData::NonVolatileData() {
	static_assert(sizeof(Theta_sens_typeE2) == 16U);
}

ErrorCode NonVolatileData::setStationId(uint32_t stationId) {
	uint32_t stationIdE2;
	AT24Cxxx::read(STATION_ID_ADDRESS, reinterpret_cast<uint8_t*>(&stationIdE2),
			sizeof(uint32_t));

	if (stationId != stationIdE2) {
		AT24Cxxx::write(STATION_ID_ADDRESS,
				reinterpret_cast<uint8_t*>(&stationId), sizeof(uint32_t));
		return ERR_OK;
	}
	return ERR_OK_E2_NOT_WRITTEN;
}
uint32_t NonVolatileData::getStationId(void) {
	uint32_t stationId;
	AT24Cxxx::read(STATION_ID_ADDRESS, reinterpret_cast<uint8_t*>(&stationId),
			sizeof(uint32_t));
	return stationId;
}

ErrorCode NonVolatileData::eraseIdTable(void) {
	Theta_sens_typeE2 idE2Data;
	idE2Data.sensorIdHash = 0xFFFFFFFF;
	idE2Data.minVal = 0xFF;
	idE2Data.maxVal = 0xFF;
	idE2Data.sensType = 0xF;
	idE2Data.relayNr = 0xF;
	for(uint8_t i=0; i < ID_Table::SHORTNAME_LEN; i++){
		idE2Data.shortname[i] = 0xFF;
	}
	idE2Data.checkSum = 0xFF;

	_currAddress = ID_TABLE_START;
	do {
		AT24Cxxx::write(_currAddress, reinterpret_cast<uint8_t*>(&idE2Data),
				sizeof(Theta_sens_typeE2));
	} while (incCurrAddress() == false);
	return ERR_OK;
}

void NonVolatileData::printIdTableRaw(void) {
	Theta_sens_typeE2 idE2Data;
	_currAddress = ID_TABLE_START;

	idE2Data = iter();
	while (!dataIsEmpty(idE2Data)) {
		uint8_t chkSum = calcChkSum(idE2Data);
		std::string shortname(idE2Data.shortname, ID_Table::SHORTNAME_LEN);
		tx_printf("%u %i %i %i %i, %s, %i \n", idE2Data.sensorIdHash,
				idE2Data.minVal, idE2Data.maxVal, idE2Data.sensType,
				idE2Data.relayNr, shortname.c_str(),
				(chkSum == idE2Data.checkSum));
		idE2Data = iter();
	}
}
/*
 * Looks through the ID-Table section, if a given ID exists. If the hash wasn't found,
 * the sensorIdHash is set to zero in the returned struct.
 */
ID_Table::Theta_sens_type NonVolatileData::getIdTableData(
		uint32_t sensorIdHash) {
	Theta_sens_typeE2 idE2Data;
	ID_Table::Theta_sens_type idPhysData;
	idPhysData.sensorIdHash = UINT32_MAX;

	_currAddress = ID_TABLE_START;
	idE2Data = iter();
	while (!dataIsEmpty(idE2Data)) {
		if ((idE2Data.checkSum == calcChkSum(idE2Data))
				&& (idE2Data.sensorIdHash == sensorIdHash)) {
			idPhysData = e2ToPhys(idE2Data);
			break;
		}
		idE2Data = iter();
	}
	return idPhysData;
}
/*
 * Looks through the ID-Table section, if a given ID exists. The data is only renewed,
 * if it's different to idPhysData. If there is no matching ID, the data is written
 * to the first free address.
 */
ErrorCode NonVolatileData::writeIdTableData(
		ID_Table::Theta_sens_type idPhysData) {
	Theta_sens_typeE2 idE2Data;
	Theta_sens_typeE2 idE2ToWrite = physToE2(idPhysData);

	// sets _currAddress
	findSensIdHashOrEmpty(idE2ToWrite.sensorIdHash);
	AT24Cxxx::read(_currAddress, reinterpret_cast<uint8_t*>(&idE2Data),
			sizeof(Theta_sens_typeE2));

	if (compareIdTableDatum(idE2Data, idE2ToWrite) == false) {
		AT24Cxxx::write(_currAddress, reinterpret_cast<uint8_t*>(&idE2ToWrite),
				sizeof(Theta_sens_typeE2));
		return ERR_OK;
	}

	return ERR_OK_E2_NOT_WRITTEN;
}
/*
 * Returns the E2-address, if a match was found, otherwise the address of
 * the first empty slot.
 * The End of the valid data-block is recognized, by finding a UINT32_MAX.
 * So, initial the E2 must be erased by writing 0xFF to the whole device.
 */
void NonVolatileData::findSensIdHashOrEmpty(uint32_t sensorIdHash) {
	Theta_sens_typeE2 idE2Data;
	_currAddress = ID_TABLE_START;
	idE2Data = iter();

	while (!dataIsEmpty(idE2Data)) {
		if (sensorIdHash == idE2Data.sensorIdHash) {
			// here, iter points to the next data entry
			decCurrAddress();
			return;
		}
		if (_currAddress >= ID_TABLE_START + ID_TABLE_LEN) {
			return;
		}
		idE2Data = iter();
	}
	return;
}
/**
 * Reads contents at _currAddress, and increases _currAddress, if content is not empty.
 */
NonVolatileData::Theta_sens_typeE2 NonVolatileData::iter(void) {
	Theta_sens_typeE2 idE2Data;
	bool incResult = false;
	AT24Cxxx::read(_currAddress, reinterpret_cast<uint8_t*>(&idE2Data),
			sizeof(Theta_sens_typeE2));
	if (!dataIsEmpty(idE2Data)) {
		incResult = incCurrAddress();
	}
	if (incResult) {
		idE2Data.sensorIdHash = 0xFFFFFFFF;
	}
	return idE2Data;
}

Convert NonVolatileData::getConversion(ID_Table::SensorType sensType) {
	Convert conv;
	if (sensType == ID_Table::SensorType::TEMP) {
		conv.setLimits(convTheta);
	} else if (sensType == ID_Table::SensorType::HUMIDITY) {
		conv.setLimits(convHumid);
	} else if (sensType == ID_Table::SensorType::PRESS) {
		conv.setLimits(convPress);
	}
	return conv;
}

NonVolatileData::Theta_sens_typeE2 NonVolatileData::physToE2(
		ID_Table::Theta_sens_type idPhysData) {
	Theta_sens_typeE2 result;
	result.sensorIdHash = idPhysData.sensorIdHash;
	result.relayNr = idPhysData.relayNr;
	result.sensType = idPhysData.sensType;
	memcpy(result.shortname, idPhysData.shortname, ID_Table::SHORTNAME_LEN);

	Convert conv = getConversion(idPhysData.sensType);
	result.minVal = conv.phys2Int(idPhysData.minVal);
	result.maxVal = conv.phys2Int(idPhysData.maxVal);
	result.checkSum = calcChkSum(result);

	return result;
}

ID_Table::Theta_sens_type NonVolatileData::e2ToPhys(
		NonVolatileData::Theta_sens_typeE2 idE2Data) {
	ID_Table::Theta_sens_type result;

	uint8_t chkSum = calcChkSum(idE2Data);
	if (chkSum != idE2Data.checkSum) {
		result.sensorIdHash = 0;
	} else {
		result.sensorIdHash = idE2Data.sensorIdHash;
	}
	result.relayNr = idE2Data.relayNr;
	result.sensType = static_cast<ID_Table::SensorType>(idE2Data.sensType);
	memcpy(result.shortname, idE2Data.shortname, ID_Table::SHORTNAME_LEN);

	Convert conv = getConversion(
			static_cast<ID_Table::SensorType>(idE2Data.sensType));
	result.minVal = conv.int2Phys(idE2Data.minVal);
	result.maxVal = conv.int2Phys(idE2Data.maxVal);

	return result;
}

bool NonVolatileData::compareIdTableDatum(Theta_sens_typeE2 tableIDLeft,
		Theta_sens_typeE2 tableIDRight) {

	uint8_t chkSumLeft = calcChkSum(tableIDLeft);
	uint8_t chkSumRight = calcChkSum(tableIDRight);

	// cross-check values
	if ((tableIDLeft.checkSum != chkSumRight) || (chkSumLeft != chkSumRight)
			|| (tableIDRight.checkSum != chkSumLeft)) {
		return false;
	}
	return true;
}

uint8_t NonVolatileData::calcChkSum(Theta_sens_typeE2 idTableDatum) {
	idTableDatum.checkSum = 0;
	uint8_t hashCode = CrcSocket::calc_chksum(
			reinterpret_cast<uint8_t*>(&idTableDatum),
			sizeof(Theta_sens_typeE2));
	return hashCode;
}

bool NonVolatileData::dataIsEmpty(Theta_sens_typeE2 idE2Data) {
	if (idE2Data.sensorIdHash == UINT32_MAX) {
		return true;
	}
	return false;
}

bool NonVolatileData::incCurrAddress(void) {
	if (_currAddress >= ID_TABLE_START + ID_TABLE_LEN) {
		_currAddress = ID_TABLE_START + ID_TABLE_LEN;
		return true;
	}
	_currAddress += sizeof(Theta_sens_typeE2);
	return false;
}
bool NonVolatileData::decCurrAddress(void) {
	if (_currAddress <= ID_TABLE_START) {
		_currAddress = ID_TABLE_START;
		return true;
	}
	_currAddress -= sizeof(Theta_sens_typeE2);

	return false;
}

} /* namespace NonVolDat */
