#ifndef WATCHDOG_TASK_H
#define WATCHDOG_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "sensor_task.h"

#define ALL_CHECKIN_BITS  (SENSOR_CHECKIN_BIT)

extern EventGroupHandle_t xCheckinEG;

void vWatchdogTask(void *pvParameters);

#endif
