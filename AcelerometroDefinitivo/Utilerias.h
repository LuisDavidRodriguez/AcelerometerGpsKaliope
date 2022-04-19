#pragma once
#include <Arduino.h>
#include <Time.h>


class Utilerias {
public:
    static time_t convertirHoraUTC(time_t timeUTC, int zonaHoraria) {


        time_t horaZonaHoraria;


        long tiempoAjusteZonaHoraria = zonaHoraria * 3600; //si en la zona horaria es -6 e. resultado sera -21600, si es +3 el resultado sera positivo y se añadira
        //para restarle el tiempo segun la libreria time 1 hora equivale a 3600s como queremos obtrener un timezone-6
        //es decir menos 6 horas -6*3600 = -21600
        //de esa forma en nuestro puerto serie tendremos
        //10/1/2020 4:19:20             //esta seria la hora y fecha utc dada por el gps
        //9 / 1 / 2020 22:19 : 20       //y esta la hora y fecha actual de la zona -6
        horaZonaHoraria = timeUTC + tiempoAjusteZonaHoraria;




        /*
        Serial.print(day(timeUTC));
        Serial.print(+"/");
        Serial.print(month(timeUTC));
        Serial.print(+"/");
        Serial.print(year(timeUTC));
        Serial.print(" ");
        Serial.print(hour(timeUTC));
        Serial.print(+":");
        Serial.print(minute(timeUTC));
        Serial.print(":");
        Serial.println(second(timeUTC));
        delay(1000);//Esperamos 1 segundo


        Serial.print(day(horaZonaHoraria));
        Serial.print(+"/");
        Serial.print(month(horaZonaHoraria));
        Serial.print(+"/");
        Serial.print(year(horaZonaHoraria));
        Serial.print(" ");
        Serial.print(hour(horaZonaHoraria));
        Serial.print(+":");
        Serial.print(minute(horaZonaHoraria));
        Serial.print(":");
        Serial.println(second(horaZonaHoraria));
        delay(1000);//Esperamos 1 segundo


     */

        return horaZonaHoraria;
    }

    static String horaEnZonaHoraria(time_t timeUTC, int zonaHoraria) {
        //dd/MM/aaaa hh/mm/ss
        time_t tiempo = convertirHoraUTC(timeUTC, zonaHoraria);

        return String(day(tiempo)) + "/" + String(month(tiempo)) + "/" + String(year(tiempo)) + " " + String(hour(tiempo)) + ":" + String(minute(tiempo)) + ":" + String(second(tiempo));
    }
};