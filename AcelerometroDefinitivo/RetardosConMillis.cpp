#include "RetardosConMillis.h"
#include <Arduino.h>


RetardosConMillis::RetardosConMillis()
{
	forzar = false;
}

RetardosConMillis::RetardosConMillis(unsigned long retardo)
{
	
	
	retardoEnMillis = retardo;
	forzar = false;

	//la inicializamos con los millis a la hora que se crea el objeto
	//esto no funciona ya que cuando se crea el objeto es en el area de declaracion de variables a menos que lo hagas en el loop o setup
	//y como en la parte de declarar variables aun no corre ningun programa no hay millis que consultar
	//millisIniciales = millis();
}


void RetardosConMillis::inicializarMillis()
{
	this->millisIniciales = millis();
}

void RetardosConMillis::setRetardo(unsigned long tiempoEnMillis)
{
	this->retardoEnMillis = tiempoEnMillis;
}

bool RetardosConMillis::seHaCumplido(bool reiniciar)
{
	millisActuales = millis();				//consultamos el tiempo actual del programa


	if ((millisActuales > millisIniciales + retardoEnMillis) || forzar) {	
		
		if (reiniciar) {
			millisIniciales = millisActuales;
			/*los milis iniciales ahora valen lo que el tiempo del programa haciendo que 
			tenga que volver a transcurrir el tiempo de retardo para que vuelva a entrar al if
			es decir aqui reiniciamos el contador para que vuelva a contar el retardo
			si el usuario no quiere reinicar el contador tiene que enviar false, 
			de esta manera si no lo reinicia una ves cumplido el primer retardo
			siempre seguira entrando y retornando se ha cumplito hasta que se reinicie externamente*/
		}
		
		forzar = false;
		return true;
	}
	else {
		return false;
	}
	
}

unsigned long RetardosConMillis::getRetardo()
{

	return this->retardoEnMillis;
}

void RetardosConMillis::reiniciar()
{
	inicializarMillis();
}

void RetardosConMillis::forzarRetardo()
{
	forzar = true;
}
