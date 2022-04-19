#pragma once
#include <Arduino.h>

//creamos los metodos estaticos que manejaran los atach interrupt

class Attach{
public:
	static volatile int pulsosFlujometroTurbinaGlp;
	

	static void aumentoPulsosFlujometroTurbinaGlp();


};
