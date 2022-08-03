/*
 Name:		Acelerometro.ino
 Created:	23/01/2020 15:11:15
 Author:	david
*/



#include "RetardosConMillis.h"
#include <avr/wdt.h>  
#include "Utilerias.h"
#include "LuisdaManejoEEPROM.h"
#include "Wire.h"
#include "Pantalla.h"
#include "Flujometro.h"
//upload to bitbucket 19-04-2022


#define MI_DIRECCION_I2C 0X2  
#define MASTER_DIRECCION_I2C 0X1  

#define ACELEROMETRO_ID_CONTADORES 2
#define ACELEROMETRO_ID_CALIBRAR 10

#define PIN_DIGITAL_FLUJOMETRO 2

/*
1.0
26-04-2020
-Aumnentamos a 7.4 las medida de ges para el ges 7
-Movemos el watch dog activado al inisio del setup
-imprimimos cuando se activa un contador en el serial de manera ordenada para visualisarlo facilmente con encabezados

*/
#define VERSION_SOFT 1.0

RetardosConMillis retardoTomaMedidas(9);           //tomaremos lecuturas del acelerometro 100 veces por segundo en defaul, cada segundo
RetardosConMillis retardoReiniciarEnviarContadores(4000);           
RetardosConMillis retardoSolicitarConfiguracionesAlMaster(4000);          

RetardosConMillis retardoImprimirMedidasSerial(500);           //Para imprimir en serial las lecturas reales del acelerometro, puedes imprimirlas igual que la toma de medidas a 9ms, pero yo la pongo a 500ms para no imprimirlas a cada rato, no es necesario tanto, solo cuando depuramos algun error

RetardosConMillis contadorMargenesMinimosRotos(2000);
RetardosConMillis contadorLimeteDeConteo(10000);


Flujometro flujometro1(PIN_DIGITAL_FLUJOMETRO);       //solo usa 18 bytes


struct datosActuales{
    float xGes = 0;         //Se guarda la fuerza G obtenida usando el punto de calibracion 0g y la lectura analogica
    float yGes = 0;         //Se guarda la fuerza G obtenida usando el punto de calibracion 0g y la lectura analogica
    float zGes = 0;         //Se guarda la fuerza G obtenida usando el punto de calibracion 0g y la lectura analogica   
    int xAnalog = 0;        //Se guarda la lectura analogica  de 0 a 1023
    int yAnalog = 0;        //Se guarda la lectura analogica  de 0 a 1023
    int zAnalog = 0;        //Se guarda la lectura analogica  de 0 a 1023
    int xPunto0ges = 0;     //es el punto de calibracion 0g guardado en la eeprom
    int yPunto0ges = 0;     //es el punto de calibracion 0g guardado en la eeprom
    int zPunto0ges = 0;     //es el punto de calibracion 0g guardado en la eeprom

}lecturas;

/*

Solo es necesario saber que en cada posicion del array almacenaremos un contador que podra lleegar hasta 256 acorde con este comentario
    X > 4.0G    ->0
    X > 3.2G    ->1
    X > 2.4G    ->2
    X > 1.6G    ->3
    X > 0.8G    ->4

    X < -4.0G   ->5
    X < -3.2G   ->6
    X < -2.4G   ->7
    X < -1.6G   ->8
    X < -0.8g   ->9



    Y > 4.0G   ->10
    Y > 3.2G   ->11
    Y > 2.4G   ->12
    Y > 1.6G   ->13
    Y > 0.8G   ->14

    Y < -4.0G  ->15
    Y < -3.2G  ->16
    Y < -2.4G  ->17
    Y < -1.6G  ->18
    Y < -0.8g  ->19         


    Z > 5.0G       ->20
    Z > 4.0G        ->21
    Z > 3.5G        ->22
    Z > 3.0G        ->23
    Z > 2.5G        ->24
    Z > 2.0G        ->25
    Z > 1.6G        ->26    
    CAIDA_LIBRE_0G  ->27

    Para procurar enviar la menor cantidad de datos posibles uniremos en un solo contador por ejemplo en eje x y y al dividirlos podriamos saber de que lado se recibi el impacto
    si fue en el trasero o delantero izquierdo o derecho, pero requerimos el doble de contadores y al analizar la informacion en el servidor tambien tendremos que analizar el doble de datos
    al juntarlos solo sabremos que se dio un golpe muy fuerte en el eje x, podria ser delantero o tracero, de igual en el eje Y podria ser izquierdo o derecho

    X > 6G  X < 6g  ->4            -----> +-0.8G lo quitaremos ahora sera 6ges El indice lo dejaremos igual para no cambiar nada mas de los codigos
    X > 4.0G  X < -4.0G  ->0
    X > 3.2G  X < -3.2G  ->1
    X > 2.4G  X < -2.4G  ->2
    X > 1.6G  X < -1.6G  ->3
     0___1____2____3____4
     4  3.2  2.4  1.6  6

    Y > 6G  Y < 6g  ->9              -----> +-0.8G lo quitaremos ahora sera 6ges El indice lo dejaremos igual para no cambiar nada mas de los codigos
    Y > 4.0G  Y < -4.0G  ->5
    Y > 3.2G  Y < -3.2G  ->6
    Y > 2.4G  Y < -2.4G  ->7
    Y > 1.6G  Y < -1.6G  ->8
    5___6____7____8___9
    4  3.2  2.4  1.6  6


    CAIDA_LIBRE_0G      z>7.4G  ->17                    ------> Se quita tambien la caida libre es muy sensible y no la tomamos en cuenta cuando revizamos gps lo cambiamos a fuerza en z > 7.4
    Z > 6.0G        ->16                    ------> EN las pruebas con varios carros 1.6G se alcanza muy facilmente, lo vamos a quitar y vamos a añadir otro nivel a 6 ges
    Z > 5.0G       ->10
    Z > 4.0G        ->11
    Z > 3.5G        ->12
    Z > 3.0G        ->13
    Z > 2.5G        ->14
    Z > 2.0G        ->15                    ------>Lo cambiaremos de 2.0g a 6.5
    10___11___12____13___14____15___16___17____18
    5    4   3.5    3    2.5  6.5   6   7.5    duracionGrabacion
    
    DURACION DEL EVENTO  ->18
   
     


    X -el EJE X NATURALMENTE DEBE DE ESTAR EN 0GS PORQUE NO HAY FUERZAS PRECENTES
    -Frenado o choque frontal las fuerzas seran +G,
                en frenados bruscos se recogieron datos de 0.5 0.6 0.7ges esos no entraran, porque podria no haber problema
                en impactos de ruedas en terracerias, se recogio un dato en x de 1.1G en un




    Y -naturalmente debera estar en 0 ges,
     -Vueltas hacia la izquierda, la fuerza centrifuga jala al cuerpo a la derecha -G
    -vueltas hacia la izquierda, la fuerza centrifuga jala al cuerpo hacia la izquierda +G
    - SI el auto derrapa e impacta con las llantas de la derecha las fuerzas G seran - porque jala el cuerpo hacia la derecha
    -Si el auto derrapa e impacta con las llantas de la izquierda las fuerzas g reportadas seran
            nO HE PROBADO AUN LOS DERRAPES PERO EN LAS VUELTAS SOLO HAY GES DE 0.3, CHOQUE EN UNA BANQUETA QUE NO VI
            Y HAY FUERZAS EN Y DE 1.8G 2.7G Y EN Z DE 1.8G, Vemos que el impacto que entro justo al chasis levanto ges grandes en el eje Y mas que en Z

    Z - naturalmente debe de estar en +1G, la gravedad de la tierra
        -cuando el auto sube, porque toma un tope, las fuerzas G son positivas, hay fuerzas de 2.2G 3.7g
        -cuando el auto cae e un hoyo o se vuela un tope y cae en caida libre las fuerzas g son menores a menos 0.3 algunas veces pasa a negativo
        -no hay numeros negativos en Z mas que los de tolerancia porque para lograr numeros negativos deberia haber presente una fuerza que empuje de arriba hacia abajo el carro con fuerza la cual no hay
        -Si el auto rampea por desirlo asi cuando hagarra un tope o oyos en terraceria cae a 0ges, pero me vole un tope muy bonito redondito y solo ubo fuerza g+ es decir el carro subio pero como el tope tenia bajadita no ubo momento de caida libre y no ubo 0G detectados





Si almacenamos cada vez que se pase de un valor en ciertos ejes en variables de tipo byte
tendriamos como limite conteos de 256, lo cual al estar tomando muestras de 100 veces por segundo
en el peor de los casos donde supongamos que el eje Z esta recibiendo 0 ges como caida libre
podriamos contar hasta 2.5 segundos antes que nuestro contador de caida libre se desborde, claro eso no ocurrira
una caida libre de 0gs no puede ser reconocida por tanto tiempo a menos que el carro este volando en un barranco!

de igual manera el eje x de aceleraciones por ejemplo su nivel natural deben ser 0gs cuando se acelera los niveles no pasan de 0.5ges
para obtener fuerzas de aceleraciones a 1g nuestros carros tendrian que ser tesla o lamborghini que aceleren de 0 a 100 en 2.7segundos
de igual manera el frenado, para alcanzar esos niveles de fuerzas g en los cehvys solo tienen que ser impactos frontales o impactos traceros
o que un vehiculo impacte por atras al chevy transfiriendole asi una aceleracion tremenda.
entonces para que nuestros contadores se desvorden en el eje x deberia estar una fuerza de aceleracion presente de 1g durante 2.56seguntos
lo cual pues es muy dificil por lo tanto al usar los contadores, ademas si la fuerza es mayor a 1g se pasara al otro contador correspondiente
lo cual permite que nuestros contadores tarden mucho mas tiempo en llegar a sus niveles de desbordamiento

nos da la oportunidad de analizar datos 100 veces por segundo o mas del acelerometro, clasificarlos en los contadores correspondientes
y quizas cada 2 segundos guardar los datos de los contadores en la memoria SD, o quizas podramos guardarlos cada 5 segundos aun lo vere en las pruebas
dependera de la cantidad de conteos que se realicen por ejemplo en terraceria

esto es bueno porque sabemos que la memoria SD es lenta, tarda 20ms en escribir datos y cerrarse, durante esos 20ms no estaremos leyendo datos del acelerometro
porque arduino estara ocupado escribiendo los datos en la sd, entonces lo mejor es procurar llamar el guardado de datos el mayor tiempo posible, para procurar
escuchar el acelerometro el mayor tiempo con mayores probabilidades de que no se nos escape un impacto fuerte del auto.

de igual forma si nuestra ventana de tiempo sera de 5 segundos, en esos 5 segundos podriamos leer los contadores y calcular la alarma correspondiente.

quizas si encontramos que las cifras de los contadores no son mayores a cierto numero, o que no se a activado un contador mayor a 3 ges, lo mejor no llamar a guardar al metodo en la SD para asi enviar
al servidor solo los datos de contadores importantes para que el Servidor procese las posibles combinaciones y asi arroje las alarmas correspondientes
pero solo de los datos relevantes


Ya hice pruebas y lo maximo a lo que llego alguno de los contadores por lo regular fue el de caida libre fue de 11, con cortes cada 2 segundos, es decir guardaba ese contador en la SD
y luego los reiniciaba todos a 0. Los demas renglones superiores o inferiores tambien acomulaban 5 caidas libres etc, pero aqui se demuestra lo dificil que sera
desbordar los bytes con conteos para llegar a 256, aun si yo juntara muchos renglnes de 2segundos por ejemplo a 6s, no se alcanzaria a desbordar ningun contador


Ahora con 2 segundos de corte y guardado en la sd, los topes que se vuelan se aprecian muy bien :3 en un solo renglon de 2 segundos salen los datos del tope
pero ahora en una terraceria o por ejemplo en la bajada de las fuentes tengo 30 segundos como 15 renglones juntos con muchos ges disparados por ejemplo hay caidas libres 63 en total en esos
15 renglones,

hay pros:
hacer cortes cada 2 segundos me gusta por los eventos cortos como los topes salen reflejados en un solo renglon, o por ejemplo
si van a 80 por hora y se vuelan un tope y tratas de ubicarlo en el google heart abra mas posibilidades de encontrar el culplable
ya que a mayor velocidad mas distancia se recorre, y si hicieramos los cortes a 5 segundos tendriamos todo un lapso de 5 segundos
donde si por ejemplo vas a 100km/h son 27 metros por segundo, tendriamos que buscar casi en 100 metros lineales el culpable de la anomalia

Me gustaria que el corte se hiciera cada 5 segundos porque todo se aprobecha mas, por ejemplo el tamano total de los contadores sera hasta 256
entonces podemos aprobechar mejor esa memoria separada almacenando 63 conteos de caida libre por ejemplo que solo 10 y volver a borrar,
nos permite mantener durante 5 segundos a arduino atento al acelerometro, los datos que se almacenen en la SD sera solo un renglon de por ejemplo 50 bytes
en lugar de 3 renglones de 2 segundos de 50bytes, es decir ahorramos datos al enviarlos al servidor tambien
y me gusta porque en los eventos como terraserias para determinar si es una terraseria podemos obsermar solo 3 renglones y no 5.

no lo se tengo que pensarlo!

Tambien en las pruebas vemos que hay muchisimos renglones guardados en la SD donde todo todo es 0 es decir en ningun momento se sobrepasaron las fuerzas G
y coincide con mi manera de manejo porque ahi fue buena, en cuanto comienzo a manejar mal comienzan a aparecer los contadores
entonces una manera seria que cuando se entre a un if que condiciona las fuerzas g y reconoce que se violo un limite, se active una variable bool
que al llegar al momento de guardar la info en la SD activara el guardado, si no hay violaciones de g, simplemente no se guarda el registro

ahora el hacer esto conllevara que no se lleve un registro lineal del tiempo en la SD, sino que a cada registro que se guarde en la SD
si es cada 2segundos o cada 5 segundos, tendremos que incluir 2 datos mas, 1 que sera la inicial a la cual se violo cualquier fuerza G
y 2 sera la hora final a la cual se guardo el renglon y se reiniciaron los contadores para la siguiente medida


Ahora eso nos abre dos maneras de escanear el tiempo
1- manejar los 2 o 5 segundos lineales, es decir no importa si se detectan ges, o no, estamos haciendo cortes cada 5 segundos o 2
    es mas facil pero abre la posibilidad a que ya casi al final de los 5 segundos se reconoscan violaciones de Gs y se guarde
    ese registro de 5 segundos pero solo con violaciones de gs muy pocas ya casi al final de los 5 segundos, claro
    el siguiente registro de 5 segundos contendra ahora el conteo de los demas ges. tambien como entre ambos registros
    se interpone la escritura de la SD tendriamos la ventana de 20ms de escritura sin leer el acelerometro lo cual nos hace perder
    de a 100 muestras por segundo 2 lecturas del acelerometro.


2- Manejar los 2 o 5 segundos (pueden ser los que queramos xD aun no decido 6 u 8 etc) que inicie el timer de aumento de los contadores
    apartir de que se violo la fuerza G mas pequeña que queremos reconocer, en todos los ejes, para el x seria fuerza g >0.8g y <-0.8g
    lo mismo para el Y, y para el Z fuerza g>1.6g y g<0.3 o caida libre, en el momento de que alguno de los valores minimos se sobrepase
    se inicia el valuo de los G y se comienzan a aumentar los contadores correspondientes, el timer de 5 segundos comienza a correr

        Y ahora se abren 2 posibilidades mas

            a- el timer comienza a correr apartir de la violacion minima, y los contadores aumentan hasta que se termina el timer de 5 segundos
                en ese momento el dato se guarda en la memoria SD o se envia al Maestro por i2C.

                    Habria 2 posibles problemas 1:
                        El evento solo fue un tope o un bache, que fue muy breve quizas solo 1 segundo de datos, y gracias a ello el renglon
                        vendra marcado con un margen de tiempo inicial de 2 segundos o de 5 segundos, es decir tendremos contadores de 5 segundos
                        para solo un evento chico, como sea no afecta mucho porque solo habra 1 o 2 contadores activados, pero abrimos la brecha de 5 segundos que mencione arriba

                        O quizas se vuela el tope y despues en esos 5 segundos otro bache etc y todos los contadores nos darian la informacion de esos 5 segundos pero no podriamos
                        distinguir que cosa fue el bache el tope o la terraceria


              b- cuando se detecta la minia violacion se activa un timer limite por ejemplo 10 segundos, si llega un periodo de paz en el acelerometro donde no se activen las vilaciones minimas
                por no se 1 segundo entonces el aumento de los contadores se detiene, se guarda en la SD y recetean los contadores,

                    de esta manera si es un evento pequeño como un bache o un tope tendremos los contadores exactos de 2 o 3 segundos quizas porque en los topes se violan lo minimo y luego sigue un momento de "paz"

                    y por ejemplo si es una terraseria seguiran ocurriendo violaciones minimas, lo cual ara que el timer de 1 segundo no se active
                    y por lo tanto los contadores siguan aumentando, hasta alcanzado el timer maximo programado de 10 segundos, en ese momento
                    se guarda el archivo en la SD se reinician los contadores.

                    y asi podriamos tener un evento mas largo continuo en un solo renglon
                    claro con su tiempo inicial y tiempo final,
                    esto hara que podamos calcular de una mejor manera si fue un tope o una terraceria completa
                    necesitamos un timer maximo para que si arduino sera capas de enviar alarmas en ese momento al chofer, al cerrar el timer lo haga.
                    y ademas necesitamos guardar esos datos en la SD para asi si arduino se reinicia no perder los datos

                    Esta seria una buena opcion porque tienes el beneficio de periodos pequeños de 2 segundos y el beneficio de analizar periodos de tiempo largos


                    pero crearia un poco de revoltijo al ver la informacion en el servidor ya que los renglones seran de tiempos variables
                    como sea creo que me gusta mas esta opcion



                    O la otra es que no pongamos timer limite y que solo incrementemos los contadores pero siempre que incrementemos 1
                    validar que su valor actual no este por llegar a los 200, si es asi lo guardamos en la Sd para evitar desbordamientos
                    entonces solo guardariamos hasta que se detecte paz en los sensores por un tiempo que definamos no se quizas 2 segundos
                    o hasta que estemos en peligro de desbordar algun contador, asi podriamos obtener periodos de tiempo mas largos






Y si no nos decantamos por alguna lo mas facil es guardar cada 2 segundos, creo que arduino no podria analizar si es terraceria o tope porque tendriamos
que leer o guardar mas de 2 segundos, sino simplemente le avisa al chofer "fuerzas g detectadas"

y del analicis de todos los datos se encargaria el servidor, el tendria que ver si el tiempo de los renglones es contiguo, o ocurrio un evento de 2 segundos
de las 11:31:25 a las 11:31:27 y despues de ahi no ubo eventos hasta las 12:00:01 a 12:00:03 y otro junto 12:00:03 : 12:00:05 y asi muchos juntos mas
para que el servidor sepa si fue terraceria o fue tope tendria que leer cada renglon y determinar si son tiempos contiguos o aislados

o simplemente si no queremos complicarnos la excistencia solo saber que cada renglon es una violacion
porque sabemos que cuando se maneja bien no debe existir ningun renglon detectado con violacion minima,
y si por ejemplo hay muchos renglones detectados contiguos como terraceria al servidor no le importa
solo proyectaria los datos de los renglones en puntos rojos en el mapa, y visualmente la secretaria determinara
si hay muchos puntos contiguos en la via sabra quie es una terraceria, y si solo hay un pontito sabra que fue algo breve como tope o bache

eso nos deja con el problema numero 2, la localizacion, este arduino sabra la hora exacta porque estara sincronizado con el master pero no sabra la localizacion
la unica manera que se registre la localizacion precisa del evento es que le comunique los datos por i2c al master por eso la importancia de mantener ela array contador menor a 32 bytes
en ese momento y suponiendo que no haya problema el master los recibe los guarda en la SD registra la hora y solicita la localizacion al modulo gps para almacenar ese dato en la SD

de esta maner y viendolo asi no seria muy buena idea poner una SD en el arduino acelerometro puesto que almacenara los datos Si, no tendriamos perdidas de informacion
pero de cualquioer manera hay que enviar todos los datos de la SD al master en algun momento para que el master las envie al servidor. esno nos da el problema enviar muchos muchos bytes
por i2c para vaciar lo guardado en la sd, eliminar los archivos, etc.

pero quizas entonces aqui si valdria la pena guardar datos de contadores con tiempo mas largo a 2 segundos o tiempo variable por evento, para asi conectarnos las menores veces posibles al
master por i2c, aqune tengo entendido es mas rapido que abrir la memoria sd escribir y cerrar, y se supone que el i2c funciona con interrupcion entocnes aunque
y es solo teoria el master este ocupado haciendo algo mas, la interrupcion entrara y leera lainfo la guarda en la SD y entonces continua haciendo lo que se quedo
aunque quizas si estaba envian informacion por gprs interrumpe, guarda en la sd y cosnulta coordenadas podria dar problemas con el modulo gprs

entocnes lo mejor seria que maestro tenga el control de cuando solicitar los contadores al esclavo, podriamos usar el modo Request
para que el maestro le solicite info al esclavo, y como nuestros contadores estan limitados a 28 bytes no habria problema de enviarlos
y seria bueno proique asi el master no esta haciendo ninguna otra cosa masque recibiendo informacion y procesando la info del acelerometro

Ahora estaria la posibilidad de que como el master este ocupado nuestos contadores del aceleromero se llegaran a desbordar aunque es raro pero podri pasar
pero creo es dificil, son 256 conteos y por ejemplo en la de las fuentes cuando me aloque el contador caida libre registro
63 conteos en 13 renglones de 2 segundos que son 26 segundos y fue el contador que mas alteraciones tuvo, de igual manera este arduino tendria que guardar la hora a la que se violo el g minimo
y hasta que el arduino master este libre que en practica no sera mas de 5 segundos los que estara ocupado, le solicitara los datos al esclavo

el cual le enviara la hora inicial a la que se registro el primer violacion y el master pondra la hora final a la que se guardo el registro.

me gustaria mas que fuera por request ya que asi mantenemos el orden maestro esclavo, porque la otra es que el esclavo le envie la informacion al maestro
invirtiendo los papeles, o que si queremos llevar un tiempo variable de captura de contadores, el esclavo le escribe al maestro de que ya tiene datos listos
el maestro deberia de entrar en un modo solo cuando este desocupado en donde leera al esclavo.
no me gusta porque se vuelve multimaestor pero en ultima instancia funcionaria. y asi el esclavo tendria control de tomar lecturas variables de tiempo que las expusimos arriba.


El master tendria que evaluar si los datos del array que envio el esclavo son diferentes de 0 y asi guardarlos en la SD




*/
byte contadores[19]; //creamos un array contadores para no manejar las estructuras debido a que los nombres vuelven un poco mas complicado el imprimir los datos

bool hayPerturbacion = false;


unsigned long millisPerturbacionDetectada = 0;
unsigned long millisPerturbacionFinalizada = 0;


//#define DEBUG

// the setup function runs once when you press reset or power the board
void setup() {
    wdt_disable();

    Serial.begin(115200);
    Serial.print(F("Version de soft: ")); Serial.println(String(VERSION_SOFT));

    wdt_enable(WDTO_1S);


    analogReference(EXTERNAL);




    Wire.begin(MI_DIRECCION_I2C);
    Wire.onReceive(onReceiveI2c);
    Wire.onRequest(onRequestI2c);
    flujometro1.escucharFlujometro();

    wdt_reset();

    memset(contadores, 0, sizeof(contadores));


    //Poner la primera vez que se carge el soft
    
    //EEluisda.setCalibracionX(526);EEluisda.setCalibracionY(525);EEluisda.setCalibracionZ(497);
    


    cargarCalibraciones();


    Serial.println(F("FIN DE SETUP"));

}

void loop() {




    wdt_reset();








    if (retardoTomaMedidas.seHaCumplido(true)) {


      


            lecturas.xAnalog = analogRead(A0);
            lecturas.yAnalog  = analogRead(A1);
            lecturas.zAnalog  = analogRead(A2);


            // normal 0 cuando se frena son positivos, cuando se acelera negativos
            // 0 vuelta a la derecha positivos, vuelta a la izquierda negativos, derrape a la izq impacto lantas izq. -g derrape derecho +g
            //debe estar en 1g, si es mayor el carro subio, si es menor a 1 o 0 el carro estaba en caida libre, no puede ser menor a 0 debido a que para ello necesitaria una fuerza externa acelerando la caida del vehiculo.


            /*
            Vamos a convertir los valores que nos entregan nuestro convertidor analogico digital del sensor a fuerzas g
            usando los valores de calibracion promedios que obtubimos de este sensor

            por ejemplo el eje z su punto 0gs esta en 495, cuanto tiene +1G su valor es 559
            entonces al valor actual le restaremos 559-495 = 64 y despues el numero obtenido lo dividiremos entre la medida que sabemos que equivale a 1G
            que en mis pruebas salio 60 o 65, cabe notar que para que el conversonr analoigico digital un numero 5 podria equivaler a milivoltios una medida muy pequeña
            por eso algunas veces sale 60 otras 65, en nuestro caso tomaremos 64
            y asi 64/64 = 1G

            si en Z se ejerciera la fuerza a en negativop es decir caida libre o mas Ges en negativo seria
            lectura actual = 395 - 495 = -100 / 60 = -1.66Ges
            y tada!!! esta la magia xD


            */



            lecturas.xGes = (lecturas.xAnalog - lecturas.xPunto0ges) / 64.0;
            lecturas.yGes = (lecturas.yAnalog - lecturas.yPunto0ges) / 64.0;
            lecturas.zGes = (lecturas.zAnalog - lecturas.zPunto0ges) / 64.0;



            if (retardoImprimirMedidasSerial.seHaCumplido(true)) {
                String acelAnalogico = String(analogRead(A0)) + "," + String(analogRead(A1)) + "," + String(analogRead(A2));
                String acelAnalogicoGesFinales = String(lecturas.xGes, 2) + "," + String(lecturas.yGes, 2) + "," + String(lecturas.zGes, 2);
                Serial.println(acelAnalogico + "," + acelAnalogicoGesFinales);
            }




            if (lecturas.xGes > 6.0 || lecturas.xGes < -6.0) {
                contadores[4]++;
            }
            else if (lecturas.xGes > 4.0 || lecturas.xGes < -4.0) {
                contadores[0]++;
            }
            else if (lecturas.xGes > 3.2 || lecturas.xGes < -3.2) {
                contadores[1]++;
            }
            else if (lecturas.xGes > 2.4 || lecturas.xGes < -2.4) {
                contadores[2]++;
            }
            else if (lecturas.xGes > 1.6 || lecturas.xGes < -1.6) {
                contadores[3]++;
            }
            /*
            else if (lecturas.xGes > 0.8 || lecturas.xGes < -0.8) {
                contadores[4]++;
            }*/


            if (lecturas.yGes > 6.0 || lecturas.yGes < -6.0) {
                contadores[9]++;
            }
            else if (lecturas.yGes > 4.0 || lecturas.yGes < -4.0) {
                contadores[5]++;
            }
            else if (lecturas.yGes > 3.2 || lecturas.yGes < -3.2) {
                contadores[6]++;
            }
            else if (lecturas.yGes > 2.4 || lecturas.yGes < -2.4) {
                contadores[7]++;
            }
            else if (lecturas.yGes > 1.6 || lecturas.yGes < -1.6) {
                contadores[8]++;
            }
            /*
            else if (lecturas.yGes > 0.8 || lecturas.yGes < -0.8) {
                contadores[9]++;
            }*/



            /*
            Lecturas reales del acelerometro con osciloscopio el voltaje en eje Z de 1G
            Por G son 200mv
            en 1g son 1.75V
            si dividimos los 1023 / 16ges son 64 unidades de analog read por G
            Escalas de voltaje apartir del 1.75v
            Si sube 300mv-2.5g      94
            400mv-3                 132
            500mv-3.5               160
            600mv-4                192
            800mv-5                256
            1000mv-6                320
            1100mv-6.5              352
            1200mv-7                384
            1300mv-7.5              416

            pero el voltaje sube hasta 1640mv que hacemos con eso lo estamos desperdiciando, deecho pasa algo porque no se esta aprovechando linialmente el acelerometro, bueno para aprovechar
            ese espectro de voltaje vamos a subir en lugar de 7 y ges seran 7.25 ges
            */

            if (lecturas.zGes > 7.4) {
                contadores[17]++;
            }
            else if (lecturas.zGes > 6.5) {
                contadores[15]++;
            }
            else if (lecturas.zGes > 6.0) {
                contadores[16]++;
            }            
            else if (lecturas.zGes > 5.0) {
                contadores[10]++;
            }
            else if (lecturas.zGes > 4) {
                contadores[11]++;
            }
            else if (lecturas.zGes > 3.5) {
                contadores[12]++;
            }
            else if (lecturas.zGes > 3) {
                contadores[13]++;
            }
            else if (lecturas.zGes > 2.5) {
                contadores[14]++;
            }
            /*
            else if (lecturas.zGes > 2.0) {
                contadores[15]++;
            }
                       
            else if (lecturas.zGes > 1.6) {
                contadores[16]++;
            }




            if (lecturas.zGes < 0.25) {
                contadores[17]++;
            }*/










            /*
            Configuramos los margenes minimos para saber si los minimos de contadores se activaron y comenzar los relojes
            Es decir mientras ninguna perturbacion en un eje se active, esta claro que todos los contadores estaran en 0
            por no tanto no habria nada que "grabar"

            De aqui quitamos lecturas.zGes < 0.25 ||
            era la caida libre la borramos

            */

            if (lecturas.zGes > 2.5 || lecturas.yGes > 1.6 || lecturas.yGes < -1.6 || lecturas.xGes > 1.6 || lecturas.xGes < -1.6) {

                if (!hayPerturbacion) {
                    contadorLimeteDeConteo.inicializarMillis();         //inicializamos nuestro contador de 10 segundos en cuanto se detecta la primera perturbacion
                    millisPerturbacionDetectada = millis();             //guardamos el tiempo al que se detecto la primera perturbacion, aqui no se volvera a etnrar solo la primera vez, se entra hasta que se reinicia a false hay perturbacion
                }


                hayPerturbacion = true;
                contadorMargenesMinimosRotos.inicializarMillis();      

                millisPerturbacionFinalizada = millis();                
                /*
                
                si deja de haber perturbaciones el tiempo se dejara de actualizar y cuando se llege el "envio de datos" sabremos en que millis se detecto la primea perturvacion
                y en que tiempo se dejaro de registrar perturvaciones, y asi sabrdemos la duracion total del evento de perturbaciones
                */
            }












            if (hayPerturbacion) {

                if (contadorMargenesMinimosRotos.seHaCumplido(true) || contadorLimeteDeConteo.seHaCumplido(true)) {
                    /*
                    Si se ha registrado una perturbacion, y ha pasado mas de 1.5segundos desde que la ultima perturbacion reinicio nuestro timer es decir fue detectada
                    entonces en ese momento dejamos de incrementar los contadores y entonces enviamos al arduino principal nuestros contadores.
                    tambien si se han seguido detectando muchas violaciones y todas dentro del tiempo de los 1.5 segundos entonces nunca entrariamos aqui y correriamos el riesgo que neustros
                    contadores se desborden, no se es tehoria yo siento que por mucho que manejes feo en terraceria habra algun momento que habra 1.5segundos de paz
                    pero bueno por si las moscas si se siguen detectando perturbaciones y alcanzamos los 10 segundos de conteo entonces enviamos los datos al arduino principal

                   
                    */

                    

                    //calculamos la duracion total del evento, como esto lo guardaremos en una variable tipo byte, que almacena numeros menores a 256 y enteros 1 2 3 4 5 6, 
                    //entonces dividiremos los millis totales del evento por ejemplo si nos dice que fueron 7500 millis, lo dividiremos entre 1000 para obtrener segundos
                    //y tendriamos 7.5S al meterlo tipo byte se tructara lo cual nos dejara 7 en la varaible byte, lo cual es mas que suficente, para los oficinistas
                    //no tiene mayor relevancia que durara 7.5 segudnos a 7segundos
                    //si el numero de duracion en bytes llega en 0 significa que el evento duro menos de 1 segundo ya que al truncar 500ms o 0.5 segundos se trunca en 0
                    

                    contadores[18] = (byte)((millisPerturbacionFinalizada - millisPerturbacionDetectada) / 1000);

                    Serial.println(String(millisPerturbacionDetectada));
                    Serial.println(String(millisPerturbacionFinalizada));
                    Serial.println(imprimirContadores());


                    /*
                    Si al enviar la informacion por i2c nos devuelve que el master no recibio la informacion entonces no reiniciamos los contadores y tambpo la variable perturbacion
                    de esta manera los contadores seguiran y seguiran aumentando si se detectan perturbaciones, entraremos a esto al ritmo que el retardo de 2 segundos de "paz" se active porque es el de menor duracion
                    o si hay muchas perturbaciones este no se activara y hasta que se active el de 10 segundos de limite volveremos a intentar el envio de datos,
                    cada que pasen 2 segundos desde la ultima perturbacion estaremos entrando aqui intentando enviar de vuelta los contadores al master si no se puede seguiremos acomulando estimulos en los contadores, asi hasta
                    que se envie la informacion al master en ese momento se reiniciaran los contadores 
                    de esa manera se supone que el byt que almacena la duracion total del evento solo puede llegar hasta el nivel del retardo limite que es de 10 segundos
                    pero si por algo no se enviaron los datos, el tiempo de duracion seguira aumentando de tal manera que podriamos tener 12s 15s 20s de duracion etc

                    El otro problema quizas de hacer esto es que los contadores se desborden que pasen su limite de 255 en por ejemplo 20 segundos, sinceramente no creo que un solo contador
                    pueda llegar hasta 255 por ejemplo 255 caidas libres en 20 segundos o 255 fuerzas mayores en x de 0.8ges, pero si eso ocurriera los datos llegarian raros porque al desbordar 255
                    se reinician a 0. pero lo repito no creo que en 20 0 30 segundos que se podria extender exagerando el periodo de que no se han podido enviar los datos, se desborden los contadores
                    Obiamente si tienes el sensor en la mano y lo volteas de cabeza en 1 segundo tendiras 100 conteos de graedad 0, pero a ver voltea el carro de cabeza?
                    EL unico invonveniente de que se extienda tantos segundos el envio de datos es que al recibirlos el master, los guardara en la SD con la posicion actual de localizacion
                    la cual claro esta sera 30 segundos mas adelante de lo real, pero bueno solo es pensar muy bien esta es la posicion en la que se guardo la info la duracion fue de 30 segundos
                    entonces de esa posicion hacia atras en un margen de 30 segundos ocurrieron las perturbaciones
                    Ya fue probado y funicona excelente!!! ahora por ejemplo tambien es bueno porque me preocupa una cosa, cuando el maestro recibe la interrupcion del i2c, dentro de esa misma interrupcion
                    estoy abriendo la sd guardando y cerrando, pero como sabemos las interrupcoones detienen al procesador de una tarea, que pasa si por ejemplo el procesador
                    tiene abierto el archivo acelerometro porque esta enviando los datos al servidor, en ese momento entra la interrupcion esta interrupcion abre y cierra el archivo
                    entonces que ocurre cuando el procesador retoma su tarea de enviar los datos que encuentra el archivo ya cerrado. o no se otro peor que este abriendo el archivo de localizacion
                    en ese moemnto entra la interrupcion abre y cierra el del acelerometro y continua con el de localizacion en un momento abra 2 archivos abiertos al mismo tiempo
                    podria eso afectar y dañar los archivos de la SD? o de alguna manera cuando el arduino esta usando el SPI detendra las interrupciones?
                    o quizas si no es asi lo mejor seria cada que se tiene abierta la SD, "desabilitar el Wire o el i2c" lo unico que pasara es que este arduino detectara que los datos
                    no se enviaron y seguira acomulando lecturas, hasta que se envien. bueno hay que pensar esto.

                    Ya despues de un rato la memoria SD comenso a dañarse me toco formatearla y ocurrian errores raros
                    cuando el arduino maestro entra al bucle que lee la SD para enviar datos a la red, desactiva las interrupciones wire, en ese momento este arduino dice que los datos
                    no se pudieron enviar y seguimos contando, cuando el arduino maestro deja de usar la SD vuelve a activar las interrupciones Wire y en ese momento este arduino 
                    indica que ya envio los datos. Wiere.end(), Wire.begin()

                    

                    quizas de esa manera nos evitamos posibles perdidas de informacion por ejemplo si se volaron un tope, se registra en los contadores se envian los datos por i2c al master pero si 
                    el master esta en un reinicio por ejemplo porque el watch dog lo reinicio, entonces no recibira los datos, y si este arduino reseteara los contadores 
                    la informacion del tope volado para cuando el master este en linea otra vez se habra perdido.
                    */
                    if (enviarByteArray()) {
                        hayPerturbacion = false;
                        memset(contadores, 0, sizeof(contadores));
                    }
                    else {
                        Serial.println(F("No se enviaron los eventos al master, no se reinician contadores, seguimos acomulando eventos hasta que se puedan enviar"));
                    }
                    




                    

                }

            }
           
           






     



    }










    /*
    Si se detectan revoluciones en el acelerometro desde la ultima vez que se reinicio a 0 entraremos a un bucle para mantener el procesador
    concentrado en leer el acelerometro, en cada bucle while se calculan los datos del acelerometro, y este metodo a su vez retorna true
    usandose para mantenerse dentro del bucle while mientras se esten recibiendo entrada de glp en el flujometro, cuando no haya entrada de gas lp
    el metodo seguira retornando true hasta que pasen 10 segundos de la ultima entrada y al fin retornara false, saliendo del bucle while 
    y dejando el procesador libre para el trabajo del acelerometro.

    Ahora se que no es bueno usar delays porque basicamente el procesador se queda atorado y no lo dejas hacer mas tareas, pero debido a que 
    es lo que queremos, con el bucle while queremos que el procesador al detectar flujo no haga nadamas mas que leer el flujometro
    entonces creo que para mas facil usaremos un delay dentro del while este no nos afectara y definira el tiempo al que se calcularan los datos
    tanto de velocidad, el tamaño maximo del delay que podremos usar dependera de nuestro watch dog ya que al estar en delay el procesador no llegara a la instruccion
    que reinicia el watch dog, cosa diferente si usaramos la clase retardoConMillis, pero para nuestra aplicacion creo que esta muy bien es mas basico y claro

    Aunque el procesador este en un delay, las interrupciones se siguen contando sin problemas, durante esos 500ms en la variable estatica se tendran el numero total de interrupciones que ocurreron
    y al llegar a calcularFlujometro se usaran para calcular los datos de litros
    */
    if (flujometro1.flujoDetectado()) {

        Serial.println(F("*****Flujo detectado en el FLujometro de GLP, entrando en bucle para leer flujometro"));

        while (flujometro1.calcularFlujometro())
        {
            /*
            Al llamarce el calcularFLujometro ahora tenemos en sus campos del objeto flujometro almacenados los datos
            del ultimo calculo, esos datos los imprimiremos en el serial
            */
            wdt_reset();
            Serial.print(F("PulsosTemporale: ")); Serial.println(String(flujometro1.getPulsosTemporales()));
            Serial.print(F("LitrosPorMinuto: ")); Serial.println(String(flujometro1.getLitrosPorMinuto()));
            Serial.print(F("PulsosTotales__: ")); Serial.println(String(flujometro1.getPulsosTotales()));
            Serial.print(F("LitrosTotales__: ")); Serial.println(String(flujometro1.getLitrosTotales()));
            Serial.println();

            delay(500);                         //contaremos las interrupciones totales cada 500ms, este valor debe de ser el mismo que se le pase a calcularFlujometro para que los datos de velocidad sean correctos

        }

        /*
        Cuando el metodo calcular flujometro detecte que ya no hay mas flujo de gas por el tubo despues de 10 segundos
        entonces retornara false saliendo del bucle while e indicando que el "Periodo de carga ha termionado"
        en esteo moemto deberiamos de consultar la variable total, pulsos totales enviarla y despues reiniciarla
        */

        Serial.println(F("*****El periodo de carga ha terminado, ya no se ha detectado flujo de GLP despues de 10 segundos, reiniciamos para un nuevo periodo de carga"));
        Serial.print(F("PulsosTotalesRegistrados: ")); Serial.println(String(flujometro1.getPulsosTotales()));
        Serial.print(F("LitrosTotalesRegistrados: ")); Serial.println(String(flujometro1.getLitrosTotales()));      
        Serial.print(F("DuracionDelPeriodo___(s): ")); Serial.println(String(flujometro1.getPeriodoCargaReal()));
        Serial.print(F("\n\n\n\n"));
        flujometro1.reiniciarPulsosTotales();

    }





}


void onReceiveI2c(int howMany) {

    Serial.print(F("Datos recibidos por el bus i2c bytes disponibles: "));
    Serial.println(String(howMany));

    byte id = 0;
    byte size = 0;

    if (howMany > 0) {
        //leemos el primer byte que por regla nuestra en el viene el id
        id = Wire.read();
        size = Wire.read();

        Serial.println("id " + String(id));
        Serial.println("size " + String(size));

        



        if (id == ACELEROMETRO_ID_CALIBRAR) {
            /*
            Si el maestro nos solicita que hagamos una calibracion de nuestro sensor acelerometro
            */
            calibrarSensorAnalogico();
            
        }


    }



}


void onRequestI2c() {
    
    Serial.println(F("\n-On Request-\n"));
    Wire.write((byte*)&lecturas, sizeof(lecturas));

}

bool enviarByteArray() {
    //enviaremos en cada transmision 1 byte del array total recibido
    //recibiremos la referencia al array que esta en la clase manejoSim
    //cada que el modulo recibe una respuesta inicializa todo el array de 100bytes a valores vacios
    //de esa manera aunque 

    Serial.println(F("\n-Enviando char Array-\n"));
    byte size = sizeof(contadores);
    Serial.print(F("Tamano de la info a enviar "));
    Serial.println(String(size));
    //Serial.println(informacion);
    //delay(50);


    wdt_reset();
    Wire.beginTransmission(MASTER_DIRECCION_I2C);
    Wire.write(MI_DIRECCION_I2C);
    Wire.write(ACELEROMETRO_ID_CONTADORES);
    Wire.write(size);
    Wire.write(contadores, sizeof(contadores));

    if (Wire.endTransmission() == 0) {
        Serial.println(F("Transmision exitosa bytes enviados: "));
        Serial.println(String(size + 2));
        return true;
    }
    else {
        Serial.print(F(" Falla en la transmision"));
        Serial.println(F("\n-Fin Enviando char Array-\n"));
        return false;
    }


          
}


void calibrarSensorAnalogico() {

    Serial.println(F("Iniciando calibracion del Sensor..."));
    Serial.println(F("Mantenga el sensor en una posicion plana y fija, ese punto se usara para definir los puntos 0g"));
    Serial.println(F("Definiendo nuevo punto 0g, X-Y-Z"));
    long sumaX = 0;
    long sumaY = 0;
    long sumaZ = 0;

    int promedioX = 0;
    int promedioY = 0;
    int promedioZ = 0;

    for (size_t i = 0; i < 10; i++)
    {
        sumaX += analogRead(A0);
        sumaY += analogRead(A1);
        sumaZ += analogRead(A2);

        delay(50);
    }

    promedioX = sumaX / 10;
    promedioY = sumaY / 10;
    promedioZ = sumaZ / 10;

    int punto0GdeZ = promedioZ - 65;
    /*
    Debido a que el eje z siempre tendra la fuerza de gravedad ejerciendo sobre el
    y con nuestras lecturas encontramos que cuando se detecta 1g en cualquier eje con el sensor calibrado a sensibilidad de 6G
    encontramos que el analog read cambia en valores de 60 aproximadamente en cada eje. en el caso del z como la fuerza g esta ejerciendo
    siempre spobre de el encontraremos una lectura del punto 0 + 60, es decir ahorita marca en el eje z una lectura de 560, su punto 0 deberia de estar entre 500 o 495
    y si inviertes el sensor es decir tendremos -1G tendremos como 430 en la lectura analogica

    el eje x y y no tienen problemas porque cuando el sensor esta en reposo no hay ninguna fuerza que incida sobre de ellos

    */

    EEluisda.setCalibracionX(promedioX);
    EEluisda.setCalibracionY(promedioY);
    EEluisda.setCalibracionZ(punto0GdeZ);


    Serial.println(F("Calibracion Finalizada Exitosamente"));
    Serial.print(F("Punto 0g de eje X= "));
    Serial.println(String(promedioX));
    Serial.print(F("Punto 0g de eje Y= "));
    Serial.println(String(promedioY));
    Serial.print(F("Punto 1g de eje Z= "));
    Serial.println(String(promedioZ));
    Serial.print(F("Punto 0g de eje Z= "));
    Serial.println(String(punto0GdeZ));
    Serial.println(F("Calibracion Finalizada Exitosamente"));



    cargarCalibraciones();

}


void cargarCalibraciones() {
    lecturas.xPunto0ges = EEluisda.getCalibracionX();
    lecturas.yPunto0ges = EEluisda.getCalibracionY();
    lecturas.zPunto0ges = EEluisda.getCalibracionZ();


    Serial.print(F("Punto 0g de eje X desde EEPROM= "));
    Serial.println(String(lecturas.xPunto0ges));
    Serial.print(F("Punto 0g de eje Y desde EEPROM= "));
    Serial.println(String(lecturas.yPunto0ges));
    Serial.print(F("Punto 0g de eje Z desde EEPROM= "));
    Serial.println(String(lecturas.zPunto0ges));
}

String imprimirContadores() {

    String dato = "\nX\n4\t3.2\t2.4\t1.6\t6\n";

    for (size_t i = 0; i < sizeof(contadores); i++)
    {
        dato += String(contadores[i]);

        if (i < sizeof(contadores) - 1) {
            dato += " ,\t";
            
            if (i == 4) dato += "\n\nY\n4\t3.2\t2.4\t1.6\t6\n";           //aqi imprimimos contadores de x
            if (i == 9) dato += "\n\nZ\n5\t4\t3.5\t3\t2.5\t6.5\t6\t7.4\tdura\n";
        }
    }


    return dato;

}


