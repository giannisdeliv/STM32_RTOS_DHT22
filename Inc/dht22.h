#ifndef DHT22_H
#define DHT22_H    // double inclusion avoidance

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* Pin definition */
#define DHT22_GPIO_PORT   GPIOA
#define DHT22_PIN         GPIO_PIN_6  // Pin we connect

/* Data struct */
typedef struct {
    float temperature;
    float humidity;
} DHT22_Data_t;        // Data struct for main()

/* Status codes */
typedef enum {
    DHT22_OK      = 0,   // returning DHT22_OK instead of 0
    DHT22_ERROR   = 1,
    DHT22_TIMEOUT = 2,
    DHT22_CRC_ERR = 3
} DHT22_Status_t;

/* Status_t */
void           DHT22_Init(void);
DHT22_Status_t DHT22_Read(DHT22_Data_t *data);
//raw data via pointer

#endif
