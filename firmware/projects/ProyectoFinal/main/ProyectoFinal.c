/*! @mainpage Proyecto final Electronica Programable
 *
 * @section genDesc General Description
 *
 * El programa permite la activacion de un vibrador dependiendo de la distancia
 * medida por el sensor de proximidad, también se puede encender o apagar esta
 * función mediante un toque del sensor de gestos.
 * Otra función es que permite la conexión bluetooth con un dispositivo externo,
 * para el envio de las distancias medidas.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral           |      ESP32                        |
 * |:-----------------------:|:----------------------------------|
 * |  Circuito del vibrador	 | 	GPIO_9		                     |
 * |        HSCR-04          |  GPIO_2 (Echo) / GPIO_3 (Trigger) |
 * |  Sensor de gestos       |  GPIO_1                           |
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 31/10/2025 | Creacion del documento                         |
 * | 14/11/2025 | Se realiza la documentacion                    |
 *
 * @author Dominguez Carmela - Lopez Ariana
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
#include "ble_mcu.h"


/*==================[macros and definitions]=================================*/
bool encendido = false;
TaskHandle_t tarea_distancia = NULL;
TaskHandle_t tarea_vibrador = NULL;


/*==================[internal data definition]===============================*/
//definicion de las distancias a las que vibra
#define DIST_MIN 20
#define DIST_MEDIA 30
#define DIST_ALTO 40
#define CONFIG_BLINK_PERIOD 500

volatile uint16_t distancia;

/*==================[internal functions declaration]=========================*/

/** 
 * @brief Define el tiempo en bajo y en alto del vibrador dependiendo de la distancia
 * @param puntero_tarea_vibrador Puntero a la tarea vibrar
*/
void vibrar(void *puntero_tarea_vibrador){
    uint16_t tiempo_ON = 0;
    uint16_t tiempo_OFF = 0;

    while (true){
        if (!encendido){
            GPIOOff(GPIO_9);
            LedOff(LED_1);
            LedOff(LED_2);
            LedOff(LED_3);
            vTaskDelay(200 / portTICK_PERIOD_MS);
            continue;
        }

        // Rango de distancia
        if (distancia > DIST_ALTO){
            // Muy lejos → no vibra
            GPIOOff(GPIO_9);
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
            GPIOOn(GPIO_9);
            vTaskDelay(tiempo_ON / portTICK_PERIOD_MS);
            GPIOOff(GPIO_9);
        }

        vTaskDelay(tiempo_OFF / portTICK_PERIOD_MS);
    }
}

/** 
 * @brief Modifica una variable cuando se toca el sensor de gestos,
 *        para definir si esta encendido o apagado
*/
void detectar_toque(){
    static bool anterior = false;
    bool actual = GPIORead(GPIO_1);

    if (actual && !anterior){
        encendido = !encendido;
    }
    anterior = actual;
}

/** 
 * @brief Mide la distancia en caso de que se encuentre encendido, envia el dato por bluetooth
 * @param puntero_tarea_distancia Puntero a la tarea medir distancia
*/
void medir_distancia(void *puntero_tarea_distancia){
    while (true){
        char msg[32];
        detectar_toque();  // Chequear si hubo toque
        if (encendido){
            distancia = HcSr04ReadDistanceInCentimeters();
            sprintf(msg, "distancia: %u\n", distancia);
            // Enviar por BLE correctamente
            BleSendString(msg);
        } else {
            distancia = 99;
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Función a ejecutarse ante un interrupción de recepción 
 * a través de la conexión BLE.
 * 
 * @param data      Puntero a array de datos recibidos
 * @param length    Longitud del array de datos recibidos
 */
void read_data(uint8_t * data, uint8_t length){


}


/*==================[external functions definition]==========================*/

void app_main(void){
    HcSr04Init(GPIO_3, GPIO_2);
    GPIOInit(GPIO_9, GPIO_OUTPUT); // Vibrador
    GPIOInit(GPIO_1, GPIO_INPUT);  // Sensor de toque
    LedsInit();
    ble_config_t ble_configuration = {
        "TIEMBLA RAYBAN",
        read_data
    };

    BleInit(&ble_configuration);
      
    xTaskCreate(&medir_distancia, "MEDIR", 4096, NULL, 5, &tarea_distancia);
    xTaskCreate(&vibrar, "VIBRAR", 4096, NULL, 5, &tarea_vibrador);

    while(1){
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        switch(BleStatus()){
            case BLE_OFF:
                LedOff(LED_1);
            break;
            case BLE_DISCONNECTED:
                LedToggle(LED_2);
            break;
            case BLE_CONNECTED:
                LedOn(LED_3);
            break;
        }
    }
}
