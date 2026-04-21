#include "sensor_task.h"
#include "dht22.h"
#include "main.h"
#include <stdio.h>

void vSensorTask(void *pvParameters)
{
    DHT22_Data_t   data;
    DHT22_Status_t status;

    vTaskDelay(pdMS_TO_TICKS(2000));

    for (;;)
    {

        /* DHT22 is timing-critical — mask scheduler tick during read */
    	 vTaskSuspendAll();
    	 status = DHT22_Read(&data);
    	 xTaskResumeAll();



        if (status == DHT22_OK)
        {
            /* Push to queue — don't block long if queue is full */
            if (xQueueSend(xSensorQueue, &data,
                           pdMS_TO_TICKS(10)) != pdTRUE)
            {
                printf("WARN: sensor queue full\r\n");
            }
        }
        else if (status == DHT22_TIMEOUT)
        {
            printf("ERROR: DHT22 timeout\r\n");
        }
        else if (status == DHT22_CRC_ERR)
        {
            printf("ERROR: DHT22 checksum\r\n");
        }

        /* IWDG check-in — signal watchdog task that we are alive */
        xEventGroupSetBits(xCheckinEG, SENSOR_CHECKIN_BIT);

        /* Yield for 2 s — DHT22 needs min 2 s between reads */
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
