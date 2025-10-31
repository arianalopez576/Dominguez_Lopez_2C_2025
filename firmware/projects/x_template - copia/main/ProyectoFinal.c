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

/*==================[macros and definitions]=================================*/

bool encendido = False;
TaskHandle_t tarea_distancia = NULL;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void activar_vibrador(uint16_t distancia){
	if (distancia < 40){
		GPIO_On(GPIO_10);
	} 
}

void detectar_toque(){
	GPIORead(GPIO_11);
	if (GPIORead){
		encendido = True;
	}
}

void medir_distancia(void *puntero_tarea_distancia){
	while (true){
		if (encendido){
			uint16_t distancia = HcSr04ReadDistanceInCentimeters();
		}
	vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}



/*==================[external functions definition]==========================*/
void app_main(void){
	HcSr04Init(GPIO_3, GPIO_2);
	//inicializacion vibrador
	GPIOInit(GPIO_10, GPIO_OUTPUT);
	//inicializacion sensor de gesto
	GPIOInit(GPIO_11, GPIO_INPUT)

	xTaskCreate(&medir_distancia, "LED_1", 512, NULL, 5, &tarea_distancia);
}
/*==================[end of file]============================================*/