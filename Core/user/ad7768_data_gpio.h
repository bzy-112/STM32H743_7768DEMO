#ifndef AD7768_DATA_GPIO_H
#define AD7768_DATA_GPIO_H

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

#define AD7768_FRAME_WORDS   8U
#define AD7768_FRAME_BYTES   32U

#define AD7768_DCLK_PORT     GPIOB
#define AD7768_DCLK_PIN      GPIO_PIN_13

#define AD7768_DOUT_PORT     GPIOB
#define AD7768_DOUT_PIN      GPIO_PIN_15

#define AD7768_DRDY_PORT     GPIOB
#define AD7768_DRDY_PIN      GPIO_PIN_9

void AD7768_DataLines_Init(void);
bool AD7768_ReadFrame(uint8_t frame[AD7768_FRAME_BYTES], uint32_t timeout_us);
bool AD7768_ReadFrameWords(uint32_t words[AD7768_FRAME_WORDS], uint32_t timeout_us);

#endif /* AD7768_DATA_GPIO_H */
