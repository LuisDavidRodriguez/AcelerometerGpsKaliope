#include "Pantalla.h"





Pantalla::Pantalla(LiquidCrystal_I2C *pan)
{
	this->pantalla = pan;
}

Pantalla::Pantalla(uint8_t direccion)
{
	pantalla = new LiquidCrystal_I2C(direccion, 16, 2);

	//pantalla es una variable de tipo puntero es decir que guardara una direccion de la memoria donde se crea el objeto
	//por eso usamos la palabra reservada new para crear un nuevo objeto pero new ya entrega una referencia es decir una 
	//direccion de la memoria donde se creo el nuevo objeto y la direcicon se almacenara en pantalla que es un puntero
	//entocnes podremos usar ambos constructores, tanto el superior que es creando el objeto liquidCristal fuera de esta clase
	//y pasandole solo su referencia de memoria, o pasando la direccion y creando aqui el objeto liquidCristal
}



void Pantalla::pantallaPrincipal(int rpm, double voltajeBateria)
{
	this->pantalla->backlight();
	//pantalla->clear();
	this->pantalla->print("Rpm: " + String(rpm) + "Vola: " + String(voltajeBateria));
}

void Pantalla::inicializar()
{
	pantalla->init();
}





void Pantalla::pantallaFlujometro(Flujometro *flujometro)
{
	pantalla->backlight();
	pantalla->clear();

	pantalla->backlight();
	pantalla->clear();
	pantalla->setCursor(0, 0);
	pantalla->print("P:" + String(flujometro->getPulsosTemporales()));
	pantalla->setCursor(6, 0);
	pantalla->print("L/m:" + String(flujometro->getLitrosPorMinuto()));
	pantalla->setCursor(11, 0);

	pantalla->setCursor(0, 1);
	pantalla->print("PT:" + String(flujometro->getPulsosTotales()));
	pantalla->setCursor(9, 1);
	pantalla->print("L:" + String(flujometro->getLitrosTotales()));
	
}


