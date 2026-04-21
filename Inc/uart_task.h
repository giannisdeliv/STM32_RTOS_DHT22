#ifndef UART_TASK_H
#define UART_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

extern QueueHandle_t    xSensorQueue;
extern SemaphoreHandle_t xUartTxDoneSem;

void vUartTask(void *pvParameters);

#endif
