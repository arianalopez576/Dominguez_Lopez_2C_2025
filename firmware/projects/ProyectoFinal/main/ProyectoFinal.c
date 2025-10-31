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

bool zona_1 = false;
bool zona_2 = false;
bool zona_3 = false;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void activar_vibrador(uint16_t distancia){
	/*
	if (distancia < 40){
		GPIOOn(GPIO_0);
	} 
	else {
		GPIOOff(GPIO_0);
	}
		*/
	if (zona_1){
		vibrar_zona1();
	}
	else if (zona_2){
		vibrar_zona2();
	}
	else if (zona_3){
		vibrar_zona3();
	}
}

void detectar_toque(){
	if (GPIORead(GPIO_1)){
		encendido = !encendido;
	}
}

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

void definir_zonas (uint16_t distancia){
	if (distancia < 10){
		zona_1 = true;
		LedOn(LED_1);
	}
	else if (distancia >= 10 && distancia < 30){
		zona_2 = true;
		LedOn(LED_2);
	}
	else if (distancia >= 30){
		zona_3 = true;
		LedOn(LED_3);
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

	//Inicializacion de timer
	timer_config_t timer_zona1 = {
        TIMER_A,
        300,
        FuncTimerA,
        NULL
    };
	TimerInit(&timer_zona1);

	xTaskCreate(&medir_distancia, "MEDIR", 512, NULL, 5, &tarea_distancia);

	TimerStart(timer_zona1.timer);
}
/*==================[end of file]============================================*/