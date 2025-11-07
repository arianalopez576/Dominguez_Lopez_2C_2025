/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * |  Resistencia	| 	GPIO_0		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "hc_sr04.h"
#include "gpio_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdbool.h"
#include "led.h"
#include "timer_mcu.h"


/*==================[macros and definitions]=================================*/
bool encendido = false;
TaskHandle_t tarea_distancia = NULL;
TaskHandle_t tarea_vibrador = NULL;


/*==================[internal data definition]===============================*/
//definicion de las distancias a las que vibra
#define DIST_MIN 20
#define DIST_MEDIA 30
#define DIST_ALTO 40

volatile uint16_t distancia;

/*==================[internal functions declaration]=========================*/

void vibrar(void *puntero_tarea_vibrador){
    uint16_t tiempo_ON = 0;
    uint16_t tiempo_OFF = 0;

    while (true){
        if (!encendido){
            GPIOOff(GPIO_0);
            LedOff(LED_1);
            LedOff(LED_2);
            LedOff(LED_3);
            vTaskDelay(200 / portTICK_PERIOD_MS);
            continue;
        }

        // Rango de distancia
        if (distancia > DIST_ALTO){
            // Muy lejos → no vibra
            GPIOOff(GPIO_0);
            LedOff(LED_1);
            LedOff(LED_2);
            LedOff(LED_3);
            tiempo_OFF = 700;
        }

        else if (distancia <= DIST_ALTO && distancia > DIST_MEDIA){
            LedOn(LED_3);
            LedOff(LED_2);
            LedOff(LED_1);
            tiempo_ON = 100;
            tiempo_OFF = 800;
        }
        else if (distancia <= DIST_MEDIA && distancia >= DIST_MIN){
            LedOn(LED_2);
            LedOff(LED_1);
            LedOff(LED_3);
            tiempo_ON = 150;
            tiempo_OFF = 500;
        }
        else if (distancia < DIST_MIN){
            LedOn(LED_1);
            LedOff(LED_2);
            LedOff(LED_3);
            tiempo_ON = 200;
            tiempo_OFF = 100;
        }

        // Vibración tipo pulso
        if (distancia <= DIST_ALTO){
            GPIOOn(GPIO_0);
            vTaskDelay(tiempo_ON / portTICK_PERIOD_MS);
            GPIOOff(GPIO_0);
        }

        vTaskDelay(tiempo_OFF / portTICK_PERIOD_MS);
    }
}

void detectar_toque(){
    static bool anterior = false;
    bool actual = GPIORead(GPIO_1);

    // Solo cambia de estado en flanco ascendente (cuando pasa de 0 a 1)
    if (actual && !anterior){
        encendido = !encendido;
    }
    anterior = actual;
}

void medir_distancia(void *puntero_tarea_distancia){
    while (true){
        detectar_toque();  // Chequear si hubo toque
        if (encendido){
            distancia = HcSr04ReadDistanceInCentimeters();
        } else {
            distancia = 99;
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

/*==================[external functions definition]==========================*/

void app_main(void){
    HcSr04Init(GPIO_3, GPIO_2);
    GPIOInit(GPIO_0, GPIO_OUTPUT); // Vibrador
    GPIOInit(GPIO_1, GPIO_INPUT);  // Sensor de toque
    LedsInit();

    xTaskCreate(&medir_distancia, "MEDIR", 512, NULL, 5, &tarea_distancia);
    xTaskCreate(&vibrar, "VIBRAR", 512, NULL, 5, &tarea_vibrador);
}
