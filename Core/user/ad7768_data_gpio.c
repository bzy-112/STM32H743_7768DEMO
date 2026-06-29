#include "ad7768_data_gpio.h"

#define AD7768_GUARD_MULTIPLIER  100U

static bool AD7768_WaitPin(GPIO_TypeDef *port,
                           uint16_t pin,
                           GPIO_PinState target,
                           uint32_t timeout_us)
{
    const uint32_t guard_limit = timeout_us * AD7768_GUARD_MULTIPLIER;
    uint32_t guard = 0;

    while (HAL_GPIO_ReadPin(port, pin) != target)
    {
        if (++guard > guard_limit)
        {
            return false;
        }
    }

    return true;
}

static bool AD7768_ShiftInBit(uint8_t *bit_out, uint32_t timeout_us)
{
    if (!AD7768_WaitPin(AD7768_DCLK_PORT, AD7768_DCLK_PIN, GPIO_PIN_RESET, timeout_us))
        return false;
    if (!AD7768_WaitPin(AD7768_DCLK_PORT, AD7768_DCLK_PIN, GPIO_PIN_SET, timeout_us))
        return false;

    *bit_out = (uint8_t)HAL_GPIO_ReadPin(AD7768_DOUT_PORT, AD7768_DOUT_PIN);
    return true;
}

void AD7768_DataLines_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef init = {0};
    init.Mode  = GPIO_MODE_INPUT;
    init.Pull  = GPIO_PULLUP;
    init.Speed = GPIO_SPEED_FREQ_LOW;

    init.Pin = AD7768_DCLK_PIN;
    HAL_GPIO_Init(AD7768_DCLK_PORT, &init);

    init.Pin = AD7768_DOUT_PIN;
    HAL_GPIO_Init(AD7768_DOUT_PORT, &init);

    init.Pin = AD7768_DRDY_PIN;
    HAL_GPIO_Init(AD7768_DRDY_PORT, &init);
}

bool AD7768_ReadFrame(uint8_t frame[AD7768_FRAME_BYTES], uint32_t timeout_us)
{
    if (frame == NULL)
        return false;

//    if (!AD7768_WaitPin(AD7768_DRDY_PORT, AD7768_DRDY_PIN, GPIO_PIN_RESET, timeout_us))
//        return false;
            while(HAL_GPIO_ReadPin(AD7768_DCLK_PORT, AD7768_DRDY_PIN) == GPIO_PIN_RESET);
            while(HAL_GPIO_ReadPin(AD7768_DCLK_PORT, AD7768_DRDY_PIN) == GPIO_PIN_SET);
    for (uint8_t word = 0; word < AD7768_FRAME_WORDS; ++word)
    {
        uint32_t acc = 0;

        for (uint8_t bit = 0; bit < 32; ++bit)
        {
            uint8_t b = 0;
            while(HAL_GPIO_ReadPin(AD7768_DCLK_PORT, AD7768_DCLK_PIN) == GPIO_PIN_SET);
            acc = (acc << 1) | HAL_GPIO_ReadPin(AD7768_DOUT_PORT, AD7768_DOUT_PIN);
             while(HAL_GPIO_ReadPin(AD7768_DCLK_PORT, AD7768_DCLK_PIN) == GPIO_PIN_RESET);
       }

        frame[word * 4 + 0] = (uint8_t)((acc >> 24) & 0xFF);
        frame[word * 4 + 1] = (uint8_t)((acc >> 16) & 0xFF);
        frame[word * 4 + 2] = (uint8_t)((acc >> 8)  & 0xFF);
        frame[word * 4 + 3] = (uint8_t)(acc & 0xFF);
    }

    return true;
}

bool AD7768_ReadFrameWords(uint32_t words[AD7768_FRAME_WORDS], uint32_t timeout_us)
{
    if (words == NULL)
        return false;

    uint8_t raw[AD7768_FRAME_BYTES] = {0};
    if (!AD7768_ReadFrame(raw, timeout_us))
        return false;

    for (uint8_t i = 0; i < AD7768_FRAME_WORDS; ++i)
    {
        words[i]  = ((uint32_t)raw[i * 4 + 0] << 24);
        words[i] |= ((uint32_t)raw[i * 4 + 1] << 16);
        words[i] |= ((uint32_t)raw[i * 4 + 2] << 8);
        words[i] |= ((uint32_t)raw[i * 4 + 3]);
    }

    return true;
}
