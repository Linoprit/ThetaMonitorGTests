/*
 * NonVolatileData.h
 *
 *  Created on: 15.04.2021
 *      Author: harald
 */

#ifndef FRAMEWORK_APPLICATION_THETASENSORS_NONVOLATILEDATA_H_
#define FRAMEWORK_APPLICATION_THETASENSORS_NONVOLATILEDATA_H_

#include <Types/AppTypes.h>
#include <Application/ThetaSensors/ID_Table.h>
#include <Libraries/Convert.h>

namespace NonVolDat {

#define	 PACKED	__attribute__ ((packed))

// Remember: invalid-value is uint8_t_max = 255!
constexpr ConvLimits convTheta { 0.1F, -12.7F, -12.7F, 12.7F };
constexpr ConvLimits convHumid { 0.156862745F, 40.0F, 40.0F, 80.0F };
constexpr ConvLimits convPress { 1.0F, 846.0F, 846.0F, 1100.0F };

class NonVolatileData {
public:
	NonVolatileData();
	virtual ~NonVolatileData() {
	}
	;
	typedef struct PACKED {
		uint32_t sensorIdHash;
		uint8_t minVal;
		uint8_t maxVal;
		uint8_t sensType :4;
		uint8_t relayNr :4; // 0 = no relay
		char shortname[ID_Table::SHORTNAME_LEN];
		uint8_t checkSum;
	} Theta_sens_typeE2;

	static Theta_sens_typeE2 iter(void);
	static ID_Table::Theta_sens_type getIdTableData(uint32_t sensorIdHash);
	static ErrorCode writeIdTableData(ID_Table::Theta_sens_type sensVals);
	static void findSensIdHashOrEmpty(uint32_t sensorIdHash);
	static uint8_t calcChkSum(Theta_sens_typeE2 idTableDatum);
	static bool compareIdTableDatum(Theta_sens_typeE2 tableIDLeft,
			Theta_sens_typeE2 tableIDRight);
	static Theta_sens_typeE2 physToE2(ID_Table::Theta_sens_type idSensValue);
	static ID_Table::Theta_sens_type e2ToPhys(Theta_sens_typeE2 e2Data);
	static Convert getConversion(ID_Table::SensorType sensType);
	static void printIdTableRaw(void);
	static bool dataIsEmpty(Theta_sens_typeE2 idE2Data);
	static ErrorCode setStationId(uint32_t stationId);
	static uint32_t getStationId(void);
	static ErrorCode eraseIdTable(void);
	static bool incCurrAddress(void);
	static bool decCurrAddress(void);

	static constexpr uint16_t ID_TABLE_START = 64U;
	static constexpr uint16_t NUM_OF_ID_ENTRIES = 100U;
	static constexpr uint16_t ID_TABLE_LEN = NUM_OF_ID_ENTRIES
			* sizeof(Theta_sens_typeE2);
	static constexpr uint16_t STATION_ID_ADDRESS = 0;

private:
	static uint16_t _currAddress;
};

} /* namespace NonVolDat */

#endif /* FRAMEWORK_APPLICATION_THETASENSORS_NONVOLATILEDATA_H_ */
