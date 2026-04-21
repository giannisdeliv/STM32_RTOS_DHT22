#include "dht22.h"
#include <stdio.h>


//  Microsecond delay with DWT cycle counter  instead of HAL delay

static void DWT_Init(void)         // private for dht22.c
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  // initialize DWT
    DWT->CYCCNT = 0;                                 // counter=0
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;           // start counting
}

static void delay_us(uint32_t us)  // private for dht22.c
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000U); //SystemCoreClock(84MHZ) / 1.000.000 = 84 cycles = 1μs
    while ((DWT->CYCCNT - start) < ticks);

}


// Pin helpers

// DHT22 uses 1-wire. so we can not have input and output at the same time

static inline void pin_output(void)  // inline-> compiler writes directly the code
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = DHT22_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(DHT22_GPIO_PORT, &GPIO_InitStruct);
}

static inline void pin_input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin  = DHT22_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT22_GPIO_PORT, &GPIO_InitStruct);
}

static inline uint8_t pin_read(void)       // read PIN
{
    return (uint8_t)HAL_GPIO_ReadPin(DHT22_GPIO_PORT, DHT22_PIN);
}


//  DHT22 Init

void DHT22_Init(void)
{
    DWT_Init();
    pin_output();   // be sure we have output
    HAL_GPIO_WritePin(DHT22_GPIO_PORT, DHT22_PIN, GPIO_PIN_SET);  // set PIN-> HIGH, sensor knows that we have no communication
    HAL_Delay(1000);  // 1 sec for next command

}

DHT22_Status_t DHT22_Read(DHT22_Data_t *data)  //MCU wakes up the sensor
{
    uint8_t  raw[5] = {0};
    uint32_t timeout = 0;

  // Signal start
    pin_output();  // We take control of the line

    HAL_GPIO_WritePin(DHT22_GPIO_PORT, DHT22_PIN, GPIO_PIN_RESET);
    HAL_Delay(1); // 1ms -> wake up signal
    HAL_GPIO_WritePin(DHT22_GPIO_PORT, DHT22_PIN, GPIO_PIN_SET);
    delay_us(15);  // DHT22 Datasheet
    pin_input();   // We drop the line




// Sensor Response
    timeout = 0;
    while (pin_read() == GPIO_PIN_SET) {    // waiting for LOW when MCU drops the line
        delay_us(1);
        if (++timeout > 100) return DHT22_TIMEOUT;
    }

    timeout = 0;
    while (pin_read() == GPIO_PIN_RESET) {  // waiting for HIGH
        delay_us(1);
        if (++timeout > 100) return DHT22_TIMEOUT;
    }

    timeout = 0;
    while (pin_read() == GPIO_PIN_SET) {    //waiting for LOW again
        delay_us(1);
        if (++timeout > 100) return DHT22_TIMEOUT;
        // no line change till 100 μsec -> something is wrong
    }


    // Read bits

        for (int i = 0; i < 40; i++) // DHT22 sends 40 bits(datasheet)
        {
            timeout = 0;
            while (pin_read() == GPIO_PIN_RESET) { // wait for LOW to end
                delay_us(1);
                if (++timeout > 100)
                	{

                	   return DHT22_TIMEOUT;

                	}

            }

            uint32_t high_time = 0;
            while (pin_read() == GPIO_PIN_SET) {
                delay_us(1);
                high_time++;                              // counts HIGH time
                if (high_time > 100) {


                      return DHT22_TIMEOUT;
                }
            }

            raw[i / 8] <<= 1;   // shift left -> new bit arrival
            if (high_time > 15) {  //safe time threshold for DHT22
                raw[i / 8] |= 1;   // write '1' on LSB
            }

        }


             // Data integrity check
                uint8_t checksum = raw[0] + raw[1] + raw[2] + raw[3];
                if (checksum != raw[4]) return DHT22_CRC_ERR;

                // DECODE Temp and HUm
                uint16_t raw_hum  = ((uint16_t)raw[0] << 8) | raw[1];         // bit15 -> sign bit
                uint16_t raw_temp = ((uint16_t)(raw[2] & 0x7F) << 8) | raw[3];

                data->humidity    = raw_hum  / 10.0f; // DHT22 send data x10
                data->temperature = raw_temp / 10.0f; // DHT22 send data x10

                if (raw[2] & 0x80) {
                    data->temperature = -data->temperature; // if MSB '-1'
                }

                return DHT22_OK;



}
