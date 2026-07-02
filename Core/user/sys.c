#include "sys.h"
#include "tim.h"
#include "hal_key.h"

extern TIM_HandleTypeDef htim6;
volatile uint32_t syscon = 0;
volatile uint32_t syscon1 = 0;
extern volatile uint16_t count[2];
extern volatile uint16_t rececount_io;
extern volatile uint16_t rececount_spi;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim6)
	{
		syscon++;
		syscon1++;
		if(syscon1 == 1000)
		{
			syscon1 = 0;
			count[0] = rececount_spi;
			count[1] = rececount_io;
			rececount_spi = 0;
			rececount_io = 0;
		}
	}
}

void task_key(void)
{
	if(syscon >= 10)
	{
		hal_KeyProc();
		syscon = 0;
	}
}
