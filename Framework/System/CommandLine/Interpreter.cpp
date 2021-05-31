/*
 * ComLineInterpreter.cpp
 *
 *  Created on: 07.11.2020
 *      Author: harald
 */

#include <System/CommandLine/Interpreter.h>
#include <Sockets/CrcSocket.h>
#include <System/serialPrintf.h>
#include <string>
#include <Application/ThetaSensors/ID_Table.h>
#include <Application/ThetaSensors/NonVolatileData.h>
#include <System/OsHelpers.h>

namespace cLine {

Interpreter::Interpreter(Lexer *dispatcher) {
}

bool Interpreter::doit(CmdBufferType comLine) {
	bool result = false;
	Lexer lex(&comLine);

	CmdToken *cmdToken = (CmdToken*) lex.getNextToken();
	if (cmdToken->getType() != Token::Command) {
		return false;
	}

	uint32_t cmd = cmdToken->getType();
//@formatter:off
	if (cmd == 2358927868U)     { result = setSensId(&lex); 		}
	else if (cmd == 3913104177) { result = getSensIdTable(&lex); 	}
	else if (cmd == 1925253141) { result = setStationId(&lex); 		}
	else if (cmd == 2948963465) { result = getStationId(&lex); 		}
	//
	// TODO
	//
	//else if (cmd == ??) { result = eraseIdTable(&lex); 		}
	// printIdTableRaw
	else if (cmd == 1676458703) { OsHelpers::SYSTEM_REBOOT();		}
//@formatter:on

	return result;
}

bool Interpreter::setStationId(Lexer *lex) {

	IntToken *intToken = (IntToken*) lex->getNextToken();
	if (intToken->getType() != Token::Integer) {
		return false;
	}
	NonVolDat::NonVolatileData::setStationId(intToken->getVal());
	return true;
}

bool Interpreter::getSensIdTable(Lexer *lex) {
	tx_printf("Printing raw data of sensor-table from E2:\n");
	NonVolDat::NonVolatileData::printIdTableRaw();
	return true;
}
bool Interpreter::getStationId(Lexer *lex) {
	uint32_t stationId = NonVolDat::NonVolatileData::getStationId();
	tx_printf("Station ID = %u\n", stationId);
	return true;
}

bool Interpreter::setSensId(Lexer *lex) {
	ID_Table::Theta_sens_type sens;

	IntToken *intToken = (IntToken*) lex->getNextToken();
	if (intToken->getType() != Token::Integer) {
		return false;
	}
	sens.sensorIdHash = intToken->getVal();

	SintToken *sintToken = (SintToken*) lex->getNextToken();
	if ((sintToken->getType() != Token::SignInt)
			|| (sintToken->getType() != Token::Integer)) {
		return false;
	}
	sens.minVal = sintToken->getVal();

	sintToken = (SintToken*) lex->getNextToken();
	if ((sintToken->getType() != Token::SignInt)
			|| (sintToken->getType() != Token::Integer)) {
		return false;
	}
	sens.maxVal = sintToken->getVal();

	intToken = (IntToken*) lex->getNextToken();
	if (intToken->getType() != Token::Integer) {
		return false;
	}
	sens.sensType = static_cast<ID_Table::SensorType>(intToken->getVal());

	intToken = (IntToken*) lex->getNextToken();
	if (intToken->getType() != Token::Integer) {
		return false;
	}
	sens.relayNr = intToken->getVal();

	ChrToken *chrToken = (ChrToken*) lex->getNextToken();
	if (chrToken->getType() != Token::String) {
		return false;
	}
	memset(sens.shortname, ' ', ID_Table::SHORTNAME_LEN);
	memcpy(sens.shortname, chrToken->getVal(), ID_Table::SHORTNAME_LEN);

	NonVolDat::NonVolatileData::writeIdTableData(sens);

	return true;
}

} /* namespace cLine */
