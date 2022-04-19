#pragma once
#include <Arduino.h>

class RetardosConMillis {
private:
	unsigned long millisIniciales;
	unsigned long millisActuales;
	unsigned long retardoEnMillis;
	bool forzar = false;

public:
	//constructor que no hace nada tendras que definir llamando a otra funcion el tiempo de retardo que quieres
	RetardosConMillis();
	//constructor que setea desde que se crea el objeto el retardo que queremos
	RetardosConMillis(unsigned long retardo);
	

	/*es necesario llamarla en el setup lo que hara sera tomar el tiempo del prosesador para
	setear correctamente los retardos, al principio lo setee directo en el constructor
	pero ocurrua que inmediatamente en cuanto iniciaba el loop el retardo estaba cumplido y se ejecutaba la instruccion
	me imagino que es porque antes del setup es la aprte de declaracion de variables ahi no hay ningun tiempo en millis de ejecucion corriendo
	*/
	void inicializarMillis();

	//si en tiempo de ejecucion del programa quieres cambiar el tiempo del retardo, por ejemplo
	//cuando por sms podamos configurar el tiempo para guardar las coordenadas definir cada cuanto queremos el envio
	void setRetardo(unsigned long tiempoEnMillis);

	//si quieres reiniciar el contador para que vuelva a comenzar su conteo envia true
	//si no quieres reiniciarlo y lo haras de manera manual en el exterior llamando a su metodo reiniciar() envia false;
	//si envias false, no se reiniciara en automatico al llamar la funcion lo que concegiras es que espere el primer retardo una vez que se cumple
	//se sigue cumpliendo infinitamente hasta que reinicies manualmente y vuelva a correr su tiempo de retardo, es util para ciertas aplicaciones
	//si envias true cada que se cumpla en automatico se reiniciara y estara esperando siempre a que se cumpla el retardo para volver a cumplirse
	//si envias true ya no necesitas reinicar() a menos que quieras extender el tiempo de retardo otra vez
	bool seHaCumplido(bool reiniciar);

	unsigned long getRetardo();
	

	/*
	Si quieres reiniciar el contador antes de que se cumpla el tiempo o si le enviaste false al metodo se ha cumplido no se reiniciara
	en automatico y tendras que llamar esta funcion para que no se este cumpliendo infinitamente una vez que se cumplio el retardo, aunque eso es fueno en ciertas funciones
	*/
	void reiniciar();


	/*
	Si necesitas que el retardo se active aunque no se haya cumplido el tiempo de espera llama a esta funcion
	*/
	void forzarRetardo();

};