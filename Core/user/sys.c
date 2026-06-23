#include "sys.h"
#include "tim.h"
#include "hal_key.h"

extern TIM_HandleTypeDef htim6;
volatile uint32_t syscon = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim6)
	{
		syscon++;
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
