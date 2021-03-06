/*
 * Unit_NonVolatileData.cpp
 *
 *  Created on: 16.04.2021
 *      Author: harald
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <EepromEmulation.h>
#include <Application/ThetaSensors/NonVolatileData.h>
#include <Devices/Eeprom_at24c256/AT24Cxxx.h>
#include <Libraries/HelpersLib.h>

using ::testing::Expectation;
using namespace NonVolDat;

void dumpPhysData(ID_Table::Theta_sens_type idPhysData) {
	std::string shortname(idPhysData.shortname, ID_Table::SHORTNAME_LEN);
	std::string sensTypeStr = ID_Table::sensorType2Str(idPhysData.sensType);
	printf("%u %.2f %.2f %s %i, %s\n", idPhysData.sensorIdHash,
			idPhysData.minVal, idPhysData.maxVal, sensTypeStr.c_str(),
			idPhysData.relayNr, shortname.c_str());
}
void dumpE2Data(NonVolatileData::Theta_sens_typeE2 idE2Data) {
	uint8_t chkSum = NonVolatileData::calcChkSum(idE2Data);
	std::string chkSumStr("_NOK_");
	if (chkSum == idE2Data.checkSum) {
		chkSumStr = "OK";
	}
	std::string shortname(idE2Data.shortname, ID_Table::SHORTNAME_LEN);
	printf("%u %i %i %i %i, %s, %s \n", idE2Data.sensorIdHash, idE2Data.minVal,
			idE2Data.maxVal, idE2Data.sensType, idE2Data.relayNr,
			shortname.c_str(), chkSumStr.c_str());
}

ID_Table::Theta_sens_type fillPhysData(uint32_t sensorIdHash, float minVal,
		float maxVal, ID_Table::SensorType sensType, uint8_t relayNr,
		std::string shortname) {
	ID_Table::Theta_sens_type idPhysData;

	idPhysData.sensorIdHash = sensorIdHash;
	idPhysData.minVal = minVal;
	idPhysData.maxVal = maxVal;
	idPhysData.sensType = sensType;
	idPhysData.relayNr = relayNr;
	memcpy(idPhysData.shortname, shortname.c_str(), ID_Table::SHORTNAME_LEN);

	return idPhysData;
}

void checkPhyEqual(ID_Table::Theta_sens_type idPhysRef,
		ID_Table::Theta_sens_type idPhysTst) {
	EXPECT_EQ(idPhysRef.sensorIdHash, idPhysTst.sensorIdHash);
	EXPECT_FLOAT_EQ(idPhysRef.minVal, idPhysTst.minVal);
	EXPECT_FLOAT_EQ(idPhysRef.maxVal, idPhysTst.maxVal);
	EXPECT_EQ(idPhysRef.sensType, idPhysTst.sensType);
	EXPECT_EQ(idPhysRef.relayNr, idPhysTst.relayNr);
	std::string refStr(idPhysRef.shortname, ID_Table::SHORTNAME_LEN);
	std::string tstStr(idPhysTst.shortname, ID_Table::SHORTNAME_LEN);
	EXPECT_TRUE(refStr == tstStr);
}

void writeSomeEntries(NonVolatileData nvd) {
	ID_Table::Theta_sens_type idPhysData;

	idPhysData = fillPhysData(4232230555U, 1.0F, 50.0F,
			ID_Table::SensorType::TEMP, 1,
			std::string("LGR_U   ", ID_Table::SHORTNAME_LEN));
	nvd.writeIdTableData(idPhysData);

	idPhysData = fillPhysData(1294211458U, -50.0F, 50.0F,
			ID_Table::SensorType::TEMP, 0,
			std::string("LGR_O   ", ID_Table::SHORTNAME_LEN));
	nvd.writeIdTableData(idPhysData);

	idPhysData = fillPhysData(3932845497U, 2.0F, 3.0F,
			ID_Table::SensorType::TEMP, 1,
			std::string("WST_U   ", ID_Table::SHORTNAME_LEN));
	nvd.writeIdTableData(idPhysData);

	idPhysData = fillPhysData(3822322055U, 1.0F, 2.0F,
			ID_Table::SensorType::TEMP, 1,
			std::string("WST_O   ", ID_Table::SHORTNAME_LEN));
	nvd.writeIdTableData(idPhysData);

	idPhysData = fillPhysData(3103548024U, -50.0F, 50.0F,
			ID_Table::SensorType::TEMP, 0,
			std::string("INNEN   ", ID_Table::SHORTNAME_LEN));
	nvd.writeIdTableData(idPhysData);

	idPhysData = fillPhysData(4003548056U, 920.0F, 1000.0F,
			ID_Table::SensorType::PRESS, 0,
			std::string("KELLER  ", ID_Table::SHORTNAME_LEN));
	nvd.writeIdTableData(idPhysData);

	idPhysData = fillPhysData(4103548028U, 950.0F, 1100.0F,
			ID_Table::SensorType::PRESS, 0,
			std::string("AUSSEN  ", ID_Table::SHORTNAME_LEN));
	nvd.writeIdTableData(idPhysData);

	idPhysData = fillPhysData(3503548028U, 65.0F, 75.0F,
			ID_Table::SensorType::HUMIDITY, 1,
			std::string("INNEN   ", ID_Table::SHORTNAME_LEN));
	nvd.writeIdTableData(idPhysData);

	idPhysData = fillPhysData(3403548056U, 67.0F, 77.0F,
			ID_Table::SensorType::HUMIDITY, 1,
			std::string("INNEN   ", ID_Table::SHORTNAME_LEN));
	nvd.writeIdTableData(idPhysData);
}

TEST(NonVolatileData, eraseIdTable) {
	EepromEmulation::init();
	EepromEmulation::fillEepromWith(0xAA);
	NonVolatileData nvd;
	writeSomeEntries(nvd);

	NonVolatileData::eraseIdTable();

	uint16_t startAddress = NonVolatileData::ID_TABLE_START;
	uint16_t endAddress = NonVolatileData::ID_TABLE_START
			+ NonVolatileData::ID_TABLE_LEN;
	for (uint16_t i = startAddress; i < endAddress; i++) {
		if (EepromEmulation::get_buffer()[i] != 0xFF) {
			FAIL() << "eraseIdTable at address " << i;
		}
	}
	//EepromEmulation::dump(0, 255);
	//printf("\n");
	//EepromEmulation::dump(endAddress - 256, endAddress + 64);
}
TEST(NonVolatileData, setStationId) {
	EepromEmulation::init();
	NonVolatileData nvd;
	writeSomeEntries(nvd);

	ErrorCode result = nvd.setStationId(123456789U);
	EXPECT_EQ(result, ERR_OK);
	uint32_t stationId_read = nvd.getStationId();
	EXPECT_EQ(123456789U, stationId_read);
	result = nvd.setStationId(123456789U);
	EXPECT_EQ(result, ERR_OK_E2_NOT_WRITTEN);
}

TEST(NonVolatileData, readWrite) {
	EepromEmulation::init();
	NonVolatileData nvd;

	ID_Table::Theta_sens_type idPhysData = fillPhysData(4232230555U, 1.0F,
			50.0F, ID_Table::SensorType::TEMP, 1,
			std::string("LGR_O   ", ID_Table::SHORTNAME_LEN));
	nvd.writeIdTableData(idPhysData);
	ID_Table::Theta_sens_type idPhysDataTst = nvd.getIdTableData(4232230555U);

	idPhysData.maxVal = 12.7F; // maxVal was saturated!
	checkPhyEqual(idPhysData, idPhysDataTst);
	// EepromEmulation::dump(64, 255);
}

TEST(NonVolatileData, readWriteMore) {
	EepromEmulation::init();
	NonVolatileData nvd;
	writeSomeEntries(nvd);

	ID_Table::Theta_sens_type idPhysDataTst;
	ID_Table::Theta_sens_type idPhysDataRef;

	idPhysDataTst = nvd.getIdTableData(3103548024U);
	idPhysDataRef = fillPhysData(3103548024U, -12.7F, 12.7F,
			ID_Table::SensorType::TEMP, 0,
			std::string("INNEN   ", ID_Table::SHORTNAME_LEN));
	checkPhyEqual(idPhysDataRef, idPhysDataTst);

	idPhysDataTst = nvd.getIdTableData(4103548028U);
	idPhysDataRef = fillPhysData(4103548028U, 950.0F, 1100.0F,
			ID_Table::SensorType::PRESS, 0,
			std::string("AUSSEN  ", ID_Table::SHORTNAME_LEN));
	checkPhyEqual(idPhysDataRef, idPhysDataTst);

	idPhysDataTst = nvd.getIdTableData(3403548056U);
	// due to numerical resolution there's a little difference to the original values
	idPhysDataRef = fillPhysData(3403548056U, 66.980392F, 77.019608F,
			ID_Table::SensorType::HUMIDITY, 1,
			std::string("INNEN   ", ID_Table::SHORTNAME_LEN));
	checkPhyEqual(idPhysDataRef, idPhysDataTst);

	idPhysDataTst = nvd.getIdTableData(3932845497U);
	idPhysDataRef = fillPhysData(3932845497U, 2.0F, 3.0F,
			ID_Table::SensorType::TEMP, 1,
			std::string("WST_U   ", ID_Table::SHORTNAME_LEN));
	checkPhyEqual(idPhysDataRef, idPhysDataTst);
}

TEST(NonVolatileData, updateId) {
	EepromEmulation::init();
	NonVolatileData nvd;
	writeSomeEntries(nvd);

	ID_Table::Theta_sens_type idPhysDataTst;
	ID_Table::Theta_sens_type idPhysDataRef;

	// before
	idPhysDataTst = nvd.getIdTableData(4103548028U);
	idPhysDataRef = fillPhysData(4103548028U, 950.0F, 1100.0F,
			ID_Table::SensorType::PRESS, 0,
			std::string("AUSSEN  ", ID_Table::SHORTNAME_LEN));
	checkPhyEqual(idPhysDataRef, idPhysDataTst);

	// after
	idPhysDataRef = fillPhysData(4103548028U, 990.0F, 1005.0F,
			ID_Table::SensorType::PRESS, 4,
			std::string("AUSSEN  ", ID_Table::SHORTNAME_LEN));
	nvd.writeIdTableData(idPhysDataRef);
	idPhysDataTst = nvd.getIdTableData(4103548028U);
	checkPhyEqual(idPhysDataRef, idPhysDataTst);

	// Check that data is not written a second time.
	EXPECT_EQ(ERR_OK_E2_NOT_WRITTEN, nvd.writeIdTableData(idPhysDataRef));
}

TEST(NonVolatileData, overFlow) {
	EepromEmulation::init();
	NonVolatileData nvd;

	ID_Table::Theta_sens_type idPhysDataRef = fillPhysData(4053548028U, 950.0F,
			1100.0F, ID_Table::SensorType::PRESS, 0,
			std::string("AUSSEN  ", ID_Table::SHORTNAME_LEN));

	for (uint8_t i = 0; i < NonVolatileData::NUM_OF_ID_ENTRIES + 5; i++) {
		idPhysDataRef.sensorIdHash++;
		nvd.writeIdTableData(idPhysDataRef);
	}
	uint16_t endAddress = NonVolatileData::ID_TABLE_START
			+ NonVolatileData::ID_TABLE_LEN
			+ sizeof(NonVolatileData::Theta_sens_typeE2) - 1;
	EXPECT_TRUE(EepromEmulation::get_buffer()[endAddress] != 0xFF);
	EXPECT_EQ(EepromEmulation::get_buffer()[endAddress + 1], 0xFF);
	//EepromEmulation::dump(endAddress - 255, endAddress + 64);
}

TEST(NonVolatileData, convPhysToE2_Temp) {
	NonVolatileData nvd;

	ID_Table::Theta_sens_type idPhysData = fillPhysData(4232230555U, -1.0F,
			50.0F, ID_Table::SensorType::TEMP, 1,
			std::string("LGR_O   ", ID_Table::SHORTNAME_LEN));

	NonVolatileData::Theta_sens_typeE2 idE2Data = nvd.physToE2(idPhysData);
	EXPECT_EQ(4232230555U, idE2Data.sensorIdHash);
	EXPECT_EQ(117U, idE2Data.minVal);
	EXPECT_EQ(254U, idE2Data.maxVal);
	EXPECT_EQ(0, idE2Data.sensType);
	EXPECT_EQ(1, idE2Data.relayNr);
	std::string idShortname(idE2Data.shortname, ID_Table::SHORTNAME_LEN);
	std::string idRef("LGR_O   ", ID_Table::SHORTNAME_LEN);
	EXPECT_TRUE(idShortname == idRef);
}
TEST(NonVolatileData, convPhysToE2_Hum) {
	NonVolatileData nvd;

	ID_Table::Theta_sens_type idPhysData = fillPhysData(4232230555U, 30.0F,
			50.0F, ID_Table::SensorType::HUMIDITY, 1,
			std::string("LGR_O   ", ID_Table::SHORTNAME_LEN));

	NonVolatileData::Theta_sens_typeE2 idE2Data = nvd.physToE2(idPhysData);
	EXPECT_EQ(4232230555U, idE2Data.sensorIdHash);
	EXPECT_EQ(0U, idE2Data.minVal);
	EXPECT_EQ(64U, idE2Data.maxVal);
	EXPECT_EQ(1, idE2Data.sensType);
	EXPECT_EQ(1, idE2Data.relayNr);
	std::string idShortname(idE2Data.shortname, ID_Table::SHORTNAME_LEN);
	std::string idRef("LGR_O   ", ID_Table::SHORTNAME_LEN);
	EXPECT_TRUE(idShortname == idRef);
}
TEST(NonVolatileData, convPhysToE2_Pres) {
	NonVolatileData nvd;

	ID_Table::Theta_sens_type idPhysData = fillPhysData(4232230555U, 920.0F,
			1200.0F, ID_Table::SensorType::PRESS, 1,
			std::string("LGR_O   ", ID_Table::SHORTNAME_LEN));

	NonVolatileData::Theta_sens_typeE2 idE2Data = nvd.physToE2(idPhysData);
	EXPECT_EQ(4232230555U, idE2Data.sensorIdHash);
	EXPECT_EQ(74U, idE2Data.minVal);
	EXPECT_EQ(254U, idE2Data.maxVal);
	EXPECT_EQ(2, idE2Data.sensType);
	EXPECT_EQ(1, idE2Data.relayNr);
	std::string idShortname(idE2Data.shortname, ID_Table::SHORTNAME_LEN);
	std::string idRef("LGR_O   ", ID_Table::SHORTNAME_LEN);
	EXPECT_TRUE(idShortname == idRef);
}
// not realy a test, but shows, how the E2-memory is organized
TEST(DISABLED_NonVolatileData, printIDTableRaw) {
	EepromEmulation::init();
	NonVolatileData nvd;
	writeSomeEntries(nvd);

	nvd.printIdTableRaw();
	EepromEmulation::dump(64, 255);
}

//dumpPhysData(idPhysData_rd);
//EepromEmulation::dump(64, 127);

