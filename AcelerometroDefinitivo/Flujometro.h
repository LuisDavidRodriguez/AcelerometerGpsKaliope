#pragma once
#include"Arduino.h"
#include "Attach.h" //añadimos la clase estatica donde estan los metodos estaticos que suman y restan la variable que contara las interrupciones
#include "RetardosConMillis.h"

class Flujometro
{


private:
	int numeroPin;
	const double mililitrosPorPulso = 1.71; // cada revolucion equivale a 2 pulsos detectados en cada revolucion pasa el doble de ml de esta cosntante

	unsigned long intervaloDeTiempo;   //almacenara el tiempo al cual lo estamos calculando por ejemplo si las estamos calculando en 1s aqui llegara 1000 para saber la velocidad del fluido
	

	
	int pulsosTemporales;				//almacenamos los pulsos que estan en la variable estatica, es temporal porque tomara un nuevo valor cuando se inicie un nuevo calculo
	unsigned long pulsosTotales;		//almacenamos toda la cantidad de pulsos que van detectados, para despues tomar esta variable y saber cuantos litros totales entraron,
	double velLitrosPorMinuto;



	unsigned long millisInicioPeriodoCargaFlujometro = 0;
	unsigned long millisFinPeriodoCargaFlujometro = 0;
	int segundosDuracionPeriodoCargaFlujometro = 0;




	unsigned long timeOutTiempoUltimaLectura = 10000;
	RetardosConMillis retardoFlujoDetectado = RetardosConMillis(timeOutTiempoUltimaLectura);
	


	RetardosConMillis retardoReiniciarFlujoDetectado = RetardosConMillis(5000);

public:
	Flujometro(int pin);	

	int getPulsosTemporales();

	void escucharFlujometro();
	void begin(unsigned long tiempoCadaCalculo, unsigned long timeOutFinalizarCarga);
	bool calcularFlujometro();



	boolean flujoDetectado();

	double getLitrosPorMinuto();

	long getPulsosTotales();

	void reiniciarPulsosTotales();

	double getLitrosTotales();

	int getPeriodoCargaReal();

	long getIntervaloTiempo();
	


	String temporalesToString();

	String totalesToString();



	
	

};


