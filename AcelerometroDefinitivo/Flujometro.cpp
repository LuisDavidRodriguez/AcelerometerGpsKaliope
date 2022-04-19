#include "Flujometro.h"


unsigned long intervaloDeTiempo = 0;
double velLitrosPorMinuto = 0;


Flujometro::Flujometro(int numeroDePin) {
	this->numeroPin = numeroDePin;
	pinMode(numeroDePin, INPUT_PULLUP);	
}










/*
Activa y setea las interrupciones en el arduino
Si no se llama este metodo no se escucharan las interrupciones nunca
*/
void Flujometro::escucharFlujometro()
{
	//creamos el elscuchador de interrupciones, que detecte los falling, y le pasamos la funcion estatica a la que llamara cuando se reconosca la interrupcion
	attachInterrupt(digitalPinToInterrupt(this->numeroPin), Attach::aumentoPulsosFlujometroTurbinaGlp ,FALLING);
}
/*
Inicializamos el flujometro
@params
long tiempoPorCalculo	->Define cada cuanto tiempo se leeran las interrupciones calculadas para obtener la velocidad del fluido
long timeOutFinalizarCarga->Define el tiempo que deba transcurrir para dar por terminado el periodo de carga despues de que no hay mas interrupciones recibidas es decir ya no hay mas glp fluyendo
*/
void Flujometro::begin(unsigned long tiempoCadaCalculo, unsigned long timeOutFinalizarCarga) {

	//creamos el elscuchador de interrupciones, que detecte los falling, y le pasamos la funcion estatica a la que llamara cuando se reconosca la interrupcion
	attachInterrupt(digitalPinToInterrupt(this->numeroPin), Attach::aumentoPulsosFlujometroTurbinaGlp, FALLING);

	this->intervaloDeTiempo = tiempoCadaCalculo;
	this->timeOutTiempoUltimaLectura = timeOutFinalizarCarga;
	retardoFlujoDetectado.setRetardo(this->timeOutTiempoUltimaLectura);
}





bool Flujometro::calcularFlujometro()
{
	//detenemos la deteccion de interrupciones para que la variable estatica deje de incrementar en lo que calculamos
	detachInterrupt(digitalPinToInterrupt(numeroPin));	


	pulsosTemporales = Attach::pulsosFlujometroTurbinaGlp;	

	double interpolacionDeRpmPorMinuto = (60000 / intervaloDeTiempo) * pulsosTemporales;
	velLitrosPorMinuto = (interpolacionDeRpmPorMinuto * mililitrosPorPulso) / 1000;

	pulsosTotales += pulsosTemporales;
	Attach::pulsosFlujometroTurbinaGlp = 0;
	attachInterrupt(digitalPinToInterrupt(this->numeroPin), Attach::aumentoPulsosFlujometroTurbinaGlp, FALLING);						//activamos denuevo las interrupciones



	/*
	Cada que calculemos nuestros datos preguntaremos si la variable que cuenta llega es diferente de 0,
	si es diferente significa que sige fluyendo gas lp por el flujometro, entonces reiniciaremos el timer
	para y retornaremos true para indicar a algun otro metodo que se calcularon los datos y que ademas hay flujo de gasLp
	cuando ya no haya flujo, seguiremos retornando true hasta que el retardo se desborde es decir pasaron 10 segundos o el tiempo programado en el timer
	desde el ultimo pulso del flujometro detectado, una ves que pasen los 10 segundos retornaremos falso indicando que hemos terminado de detectar una carga de glp
	y pudioendo dejar el procesador disponible para su siguiente tarea

	si deja de haber flujo tendra 10 segundos antes de que se finalice la carga si vuelve a haber otro pulso entonces se reinicia el timer
	*/

	if (pulsosTemporales != 0) {
		this->retardoFlujoDetectado.reiniciar();
		return true;
	}
	else {
		/*
		Si ya no hay flujo de glp seguiremos retornado true hasta que se desborde el timer
		*/

		if (retardoFlujoDetectado.seHaCumplido(true)) {
			this->millisFinPeriodoCargaFlujometro = millis();
			return false;
		}
		else {
			return true;
		}

	}


	






	
	




	/*Por ejemplo si tenemos un conteo de 10 rpm en 1 segundo, necesitamos saber aproximado
	  cuantas revoluciones son al minuto, pero tenemos que dividir el minuito en el tiempo de muestreo
	  un minuto son 60mil entre el tiempo de captura de rpm que es miln entonces queda de 60, multiplicamos 60 por las rpm capturadas
	  y nos da la escala del minuto
	  igual si el muestreo se toma en 400ms y se contaron 15rpm entonces 60mil/400 son 150 * 15rpm = 2250 rpm en ese minuto a esa velocidad
	  

	  Asignamos a todas las variables del objeto los valres capturados para que los puedamos obtener cuando lo deseemos
	  */


	
	
	
}



/*se tiene que llamar primero al metodo calcular flujometro por ejemplo 
en el programa principal de arduino se tiene que poner un timer por ejemplo 1 segundo cunado se cumple ese segundo llamas a
calcular flujometro, el cual parara la escucha de interrupciones, y toma el conteo que tuvo en el segundo hace los calculos
setea sus variables campos con los valores necesarios, y despues que acaba los calculos vuelve a escuchar las interrups
en ese momento despues de tu linea calcular flujometro tienes tus variables campo disponibles con la informacion actualizada para
mostrarlos en pantalla con los metodos de abajo, esto claro se hara dentro del mismo ciclo del timer, asi cuando se vuelva a cumplir
el timer de otro segundo los campos del flujometro se actualizaran con la nueva informacion
*/



/*
Retorna los pulsos que se obtubieron durante el periodo de tiempo que duro el calculo
si el calculo es cada 500ms retorna cuantos pulsos tenia la variable estatica en esos 500ms
*/
int Flujometro::getPulsosTemporales()
{
	return pulsosTemporales;
}


/*
Si hemos detectado pulsos retornamos true
En ocaciones a lo largo del dia el flujometro podria llegar a moverse y generar pulsos
estos pulsos a lo largo del dia podrian llegar hasta el minimo de este y hacer que 
entre en el bucle de deteccion de flujometro, el problema es que como tenemos 15 segundos despues
del ultimo pulso detectado para salir del bucle entonces escribiremos datos en valde.
*/
bool Flujometro::flujoDetectado()
{
	/*
	Si el contador de pulsos es mayor a 5 es decir desde el ultimo calculo del flujometro donde se reinicio a 0
	se han detectado nuevos pulsos en las interrupciones entonces retornamos true,

	*/



	if (Attach::pulsosFlujometroTurbinaGlp > 10) {
		millisInicioPeriodoCargaFlujometro = millis();
		this->pulsosTotales = 0;						//reiniciamos los pulsos totales con cada nueva carga registrada
		return true;
	}
	else
	{
		return false;
	}
}


/*
Retorna la velocidad obtenida del fluido en el periodo de calculo
si el tiempo de calculo es cada 500ms, te indica a que velocidad paso el fluido
durante esos 500ms
*/
double Flujometro::getLitrosPorMinuto(){
	return velLitrosPorMinuto;
}


/*
Retorna, el total de los pulsos detectados durante el periodo de carga
si cada 500ms se calculaba, los pulsos de esos 500ms se sumaban en esta variable
si el periodo de carga tardo 60 segundos en esta variable tendremos el total de pulsos
de esos 60segundos.
La variable se debera reiniciar a 0 con el metodo reiniciarValoresTotales
antes de iniciar un nuevo periodo de carga
*/
long Flujometro::getPulsosTotales()
{
	return pulsosTotales;
}


/*
Es necesario reiniciar la variable pulsos totales en el inicio de un nuevo periodo de carga
*/
void Flujometro::reiniciarPulsosTotales()
{
	pulsosTotales = 0;
}


/*
Retorna el valor total de litros cargados en el periodo de carga
no es necesario reiniciar esta variable debido a que solo calcula
de los pulsos totales se multiplica por los millilitros y devuelve los litros
es decir mientras se reinicie eliminar pulsos totales, este metodo multiplicara por 0
y devolvera 0 claro esta xD
*/
double Flujometro::getLitrosTotales()
{
	return (pulsosTotales * mililitrosPorPulso) / 1000;
}

/*
Nos entrega desde el momento en que fueron detectadas las primeras revoluciones con el metodo flujo detectado hasta
que se detectaron las ultimas revoluciones. nos da los segundos de flujo
*/
int Flujometro::getPeriodoCargaReal()
{
	long calculo = this->millisFinPeriodoCargaFlujometro - this->millisInicioPeriodoCargaFlujometro;
	calculo -= timeOutTiempoUltimaLectura;
	int periodo = (int) (calculo / 1000);
	return periodo;			//le quitamos el tiempo que espera para salir del periodo de carga y asi obtener los segundos reales de carga
}

/*
Por si necesitas saber cada cuanto tiempo esta calculando el flujo si cada 500ms o cada 1000ms
*/
long Flujometro::getIntervaloTiempo()
{
	return this->intervaloDeTiempo;
}




/*
Retornamos los datos del acelerometro del ultimo calculo en String
pulsosTemporales,LitrosPorMinuto,PulsosTotales,LitrosTotales,intervaloCalculo,TipoRenglonTemporal_FInal,Lat,long,fec,hr
59,12.11,351,0.60,500,t
retornaremos estos datos por si quieres imprimir en la memoria sd un renglon por cada calculo que haga el flujometro por ejemplo cada 500ms
*/
String Flujometro::temporalesToString()
{
	return String(getPulsosTemporales()) + "," + String(getLitrosPorMinuto()) + "," + String(getPulsosTotales()) + "," + String(getLitrosTotales()) + "," + String(getIntervaloTiempo()) + ",T";
}

/*
Retornamos los datos del flujometro finales, este se puede llamar al finalizar el periodo de carga para que entrege solo los resultados finales
pulsosTemporales,LitrosPorMinuto,PulsosTotales,LitrosTotales,duracionPerido,TipoRenglonTemporal_FInal
0,0,1954,3.34,7,F
*/
String Flujometro::totalesToString()
{
	return String(0) + "," + String(0) + "," + String(getPulsosTotales()) + "," + String(getLitrosTotales()) + "," + String(getPeriodoCargaReal()) + ",F";
}


