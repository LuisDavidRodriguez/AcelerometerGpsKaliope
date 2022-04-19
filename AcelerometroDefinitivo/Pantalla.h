#pragma once
#include <LiquidCrystal_I2C.h>
#include<Arduino.h>
#include"Flujometro.h"

//(trate de crear esta clase, para manejar las pantallas lo que queria
//que aqui se almacenara un objeto de la clase LiquidCrystal y sobre ese objeto
//mostrar todas las pantallas que quiciera, era pasarle por su constructor el objeto LiquidCristal
//al inicio marco un error, decia no existe constructor predefinido para la clase liquidCrystal
//entonces al parecer el problema era directo en la clase LiquidCrystal le di control para que me
//enviara al archivo de la clase, y ahi esta su constructor que lleva tres parametros
//pero segun el sistema faltaba uno predefinido es decir sin parametros, entonces declare otro
//constructor en el archivo de liquidCrystal y dejo de marcar el error, pero ahora comenzo a marcar
//el de Pantalla.cpp:8: undefined reference to LiquidCrystal_I2C  LiquidCrystal_I2C()
//no entiendo como resolverlo aun creo entonces que lo que hare es crear una clase Estatica, 
//y en ella le pasare a cada metodo estatico el objeto LiquidCrystal al que imprimiremos
//solo se complicara un poco porque tienes que definir el objeto liquidCristal y estarselo
//pasando a cada metodo
//pero esta funcionando en lo que entinedo como resolver este


//ya lo entendi creo
//hay que enviarle punteros es decir la posicion de la memoria donde esta guardado el objeto que se creo y se quiere pasar a esta clase
//y acceder a sus metodos tratandolo como puntero es decir con la ->)



class Pantalla{

private:
	LiquidCrystal_I2C *pantalla;
	Flujometro *flujometro;

	


public:
	Pantalla(LiquidCrystal_I2C *pan); //le pido como parametro un objeto liquidCrystal hay que crearlo en el archivo ino y pasarselo a este constructor
	Pantalla(uint8_t direccion);
	void pantallaPrincipal(int rpm, double voltajeBateria);
	void inicializar();

	void pantallaFlujometro(Flujometro* flujometro);

};