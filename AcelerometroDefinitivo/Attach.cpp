#include "Attach.h"

//guardamos aqui las funciones estaticas que se encargan de manejar las interrupciones
//porque recordar que cuando se declara el attachInterrupt no nos deja unsar funciones que 
//pertenescan a una clase de la cual se crean objetos. entonces mejor aqui creamos las funciones
//que se encargan de aumentar los contadores de interrupciones, y en la clase donde usaremos la interrupcion
//manejamos todo lo demas. buscar en internet que no es posible con la funcion attachinterrup de arduino pasarle un objeto de una clase


volatile int Attach::pulsosFlujometroTurbinaGlp = 0;
void Attach::aumentoPulsosFlujometroTurbinaGlp()
{
	Attach::pulsosFlujometroTurbinaGlp++;
}
