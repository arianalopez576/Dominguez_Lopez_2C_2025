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



/*==================[macros and definitions]=================================*/

bool encendido = false;
TaskHandle_t tarea_distancia = NULL;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void activar_vibrador(uint16_t distancia){
	if (distancia < 40){
		GPIOOn(GPIO_0);
	} 
	else {
		GPIOOff(GPIO_0);
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
			uint16_t distancia = HcSr04ReadDistanceInCentimeters();
			activar_vibrador(distancia);
		}else {
            GPIOOff(GPIO_0);
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

	xTaskCreate(&medir_distancia, "MEDIR", 512, NULL, 5, &tarea_distancia);
}
/*==================[end of file]============================================*/