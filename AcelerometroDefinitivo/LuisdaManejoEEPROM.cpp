#include "LuisdaManejoEEPROM.h"


void LuisdaManejoEEPROM::recorrerIndicesMemoriaEEPROM()
{
	for (size_t i = 0; i < 300; i++)
	{
		char pff = EEPROM.read(i);
		Serial.println("Pos: " + String(i) + " valor " + pff);
	}
}


void LuisdaManejoEEPROM::escribirByteAbyteMemoria(byte valor, int limite)
{
#ifdef DEBUG
	Serial.println("Tamano de memoria EEPROM: " + String(EEPROM.length()) + " bytes");
#endif // DEBUG

	if (limite >= EEPROM.length()) {

#ifdef DEBUG
		Serial.println(F("Se paso un valor final a escribir mayor a la memoria eeprom"));
#endif // DEBUG

		return;
	  	}

	for (size_t i = 0; i <= limite; i++)
	{
		EEPROM.write(i,valor);
	}

}


void LuisdaManejoEEPROM::setCalibracionX(int info)
{
	EEPROM.put(_EELUISDA_ADDRESS_CALIBRACION_X, info);
}

int LuisdaManejoEEPROM::getCalibracionX()
{
	int info;
	EEPROM.get(_EELUISDA_ADDRESS_CALIBRACION_X, info);


	return info;
}


void LuisdaManejoEEPROM::setCalibracionY(int info)
{
	EEPROM.put(_EELUISDA_ADDRESS_CALIBRACION_Y, info);
}

int LuisdaManejoEEPROM::getCalibracionY()
{
	int info;
	EEPROM.get(_EELUISDA_ADDRESS_CALIBRACION_Y, info);


	return info;
}


void LuisdaManejoEEPROM::setCalibracionZ(int info)
{
	EEPROM.put(_EELUISDA_ADDRESS_CALIBRACION_Z, info);
}

int LuisdaManejoEEPROM::getCalibracionZ()
{
	int info;
	EEPROM.get(_EELUISDA_ADDRESS_CALIBRACION_Z, info);


	return info;
}




