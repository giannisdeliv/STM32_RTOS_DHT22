#include "uart_task.h"
#include "dht22.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart2;

void vUartTask(void *pvParameters)
{
    DHT22_Data_t rx;
    char buf[64];

    for (;;)
    {
        if (xQueueReceive(xSensorQueue, &rx, portMAX_DELAY) == pdTRUE)
        {
            int len = snprintf(buf, sizeof(buf),                          // sprintf=format and store a string in a buffer, with built-in protection against buffer overflows
                               "Temp: %.1f C  |  Hum: %.1f %%\r\n",
                               rx.temperature, rx.humidity);

            /* Non-blocking DMA transmit */
            HAL_UART_Transmit_DMA(&huart2, (uint8_t *)buf, len);

            /* Block until TX Complete callback gives the semaphore */
            xSemaphoreTake(xUartTxDoneSem, portMAX_DELAY);
        }
    }
}

