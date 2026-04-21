#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

/* IWDG check-in bit for this task */
#define SENSOR_CHECKIN_BIT   ( 1 << 0 )

/* Shared RTOS object handles — defined in main.c, extern here */
extern QueueHandle_t      xSensorQueue;
extern EventGroupHandle_t xCheckinEG;

void vSensorTask(void *pvParameters);

#endif
