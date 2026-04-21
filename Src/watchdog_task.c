#include "watchdog_task.h"
#include "main.h"
#include <stdio.h>

extern IWDG_HandleTypeDef hiwdg;

void vWatchdogTask(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(500));

        EventBits_t bits = xEventGroupGetBits(xCheckinEG);

        if ((bits & ALL_CHECKIN_BITS) == ALL_CHECKIN_BITS)
        {
            xEventGroupClearBits(xCheckinEG, ALL_CHECKIN_BITS);
            HAL_IWDG_Refresh(&hiwdg);
        }
    }
}
