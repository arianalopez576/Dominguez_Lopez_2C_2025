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

/*==================[internal data definition]===============================*/
struct vibrador
	{
	uint8_t mode; //ON, OFF, TOGGLE
	uint8_t n_ciclos; //indica la cantidad de ciclos de encendido/apagado (parpadeo)
	uint16_t periodo; //indica el tiempo de cada ciclo
	};

#define OFF = 0; //cuando no vibra
#define ON = 1;
#define TOGGLE = 2;

/*==================[internal functions declaration]=========================*/
void activar_vibrador(uint16_t distancia){
	if (distancia < 40){
		GPIOOn(GPIO_0);
	} 
	else {
		GPIOOff(GPIO_0);
	}
	/*
	if (mi_vibrador.mode == OFF){
		GPIOOff(GPIO_0);
	}
	//----MODO TOGGLE---
	else if (mi_vibrador.mode == ON){
	}
	*/
}

void detectar_toque(){
	if (GPIORead(GPIO_1)){
		encendido = !encendido;
	}
}
/*
void definir_zonas (uint16_t distancia){
	if (distancia < 10){
		LedOn(LED_1);
		struct vibrador vibrador1;
		vibrador1.mode = TOGGLE;
		mi_led.n_ciclos = 10;
		mi_led.periodo = 500; //estos son milisegundos
	}
	else if (distancia >= 10 && distancia < 30){
		LedOn(LED_2);
	}
	else if (distancia >= 30){
		LedOn(LED_3);
	}
}

*/

void medir_distancia(void *puntero_tarea_distancia){
	while (true){
		detectar_toque();  // Chequear si hubo toque
		if (encendido){
			LedOn(LED_1); //Enciende dos Leds cuando está encendido
			LedOn(LED_2);
			uint16_t distancia = HcSr04ReadDistanceInCentimeters();
			activar_vibrador(distancia);
		}else {
            GPIOOff(GPIO_0);
			LedOff(LED_1);
			LedOff(LED_2);
		}
		vTaskDelay(200/portTICK_PERIOD_MS);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	//inicializacion hcsr
	HcSr04Init(GPIO_3, GPIO_2);
	//inicializacion vibrador
	GPIOInit(GPIO_0, GPIO_OUTPUT);
	//inicializacion sensor de gesto
	GPIOInit(GPIO_1, GPIO_INPUT);
	//inicializacion leds
	LedsInit();
/*
	//Inicializacion de timer
	timer_config_t timer_zonas = {
        TIMER_A,
        300,
        FuncTimerA,
        NULL
    };

*/

	xTaskCreate(&medir_distancia, "MEDIR", 512, NULL, 5, &tarea_distancia);
}
/*==================[end of file]============================================*/