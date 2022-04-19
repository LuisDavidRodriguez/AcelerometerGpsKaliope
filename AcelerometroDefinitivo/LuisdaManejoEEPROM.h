#pragma once
#include <Arduino.h>
#include <EEPROM.h>
class LuisdaManejoEEPROM
{
private:


#define _EELUISDA_ADDRESS_CALIBRACION_X 0		//guardaremos un INT usara 2 bytes 0-1 sigiente posicion 2
#define _EELUISDA_ADDRESS_CALIBRACION_Y 2		//guardaremos un INT usara 2 bytes 2-3 sigiente posicion 4
#define _EELUISDA_ADDRESS_CALIBRACION_Z 4		//guardaremos un INT usara 2 bytes 4-5 sigiente posicion 6



public:

	
	
	void begin();

	void recorrerIndicesMemoriaEEPROM();

	void escribirByteAbyteMemoria(byte valor, int limite);

	void setCalibracionX(int info);

	int getCalibracionX();

	void setCalibracionY(int info);

	int getCalibracionY();

	void setCalibracionZ(int info);

	int getCalibracionZ();






};



extern LuisdaManejoEEPROM EEluisda;

