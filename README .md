# STM32_RTOS_DHT22


The previous project was about a working bare-metal DHT22 driver with DWT-based microsecond timing, IWDG watchdog, and UART output. This project migrates that  to FreeRTOS, keeping the driver untouched and introducing a proper multi-task architecture.



## Hardware



Board -> STM32 Nucleo-F401RE 

Sensor -> DHT22 (temperature & humidity, single-wire)

Data pin -> PA6 with 4.7 kΩ pull-up to 3.3V 

UART/USART2 → ST-Link VCP, 115200 baud 

Watchdog -> IWDG on LSI, ~3s timeout 


*Architecture

Three concurrent FreeRTOS tasks communicate through a queue and an event group:

```
vSensorTask  (P=2) ──► xSensorQueue ──► vUartTask   (P=1)
     │                                       │
     └── xCheckinEG (event bit) ──► vWatchdogTask (P=3)
                                        │
                                   HAL_IWDG_Refresh()
```

1)vSensorTask
Reads the DHT22 every 2 seconds. The read is wrapped in `vTaskSuspendAll()` / `xTaskResumeAll()` to prevent context switches from corrupting the 1-wire timing, while keeping hardware interrupts (and the HAL tick) active. If succesful, it pushes the result to the queue and sets its check-in bit in the event group.

2)vUartTask
Blocks on the queue indefinitely. When data arrives, it formats the string and transmits via **DMA** (non-blocking). It then waits on a binary semaphore that is given by the `HAL_UART_TxCpltCallback` ISR, so the task only proceeds once the transfer is complete.

3)vWatchdogTask
Runs every 500ms at the highest priority. It inspects the event group for check-in bits from all critical tasks. Only if all bits are set does it refresh the IWDG and clear the bits. If any task misses a cycle, the watchdog is withheld and the system resets after ~3 seconds — the correct behaviour for a fault.

---

*Key technical points

1)vTaskSuspendAll() instead of taskENTER_CRITICAL()  
taskENTER_CRITICAL() raises BASEPRI, which masks the TIM1 interrupt used as the HAL timebase. This causes HAL_Delay() inside the DHT22 driver to hang indefinitely. vTaskSuspendAll() disables the scheduler without masking hardware interrupts, so the driver timing remains correct.

2)TIM1 as HAL timebase
FreeRTOS takes over SysTick for its own tick. TIM1 is assigned as the HAL timebase in CubeMX to avoid the conflict — consistent with the plan for any future I2C sensor additions.

3)IWDG refreshed only when all tasks are alive
Unlike the bare-metal predecessor where the watchdog was refreshed unconditionally in the main loop, here a dedicated high-priority task acts as a health monitor. The system only stays alive if the sensor task completes its cycle on time.



*Project structure

```
Core/
├── Inc/
│   ├── dht22.h          <- same as the previous project
│   ├── sensor_task.h
│   ├── uart_task.h
│   ├── watchdog_task.h
│   └── FreeRTOSConfig.h
└── Src/
    ├── dht22.c           <- same as the previous project
    ├── main.c
    ├── sensor_task.c
    ├── uart_task.c
    └── watchdog_task.c
```





Developed in **STM32CubeIDE**. Open the `.project` file directly. No external dependencies beyond the STM32F4 HAL and FreeRTOS middleware, both managed by CubeMX.
UART output is visible on the ST-Link virtual COM port at 115200 8N1.

