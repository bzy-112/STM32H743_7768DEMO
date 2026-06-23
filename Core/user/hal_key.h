#ifndef __HAL_KEY_H
#define __HAL_KEY_H

#include "main.h"
#define GPIO_STATE(GPIO_PORT,GPIO_PIN)		HAL_GPIO_ReadPin(GPIO_PORT,GPIO_PIN)


#define KEY0_PORT   KEY2_GPIO_Port
#define KEY0_PIN    KEY2_Pin

#define KEY1_PORT   GPIOC
#define KEY1_PIN    GPIO_PIN_7

#define KEY2_PORT   GPIOC
#define KEY2_PIN    GPIO_PIN_8

#define KEY3_PORT   GPIOC
#define KEY3_PIN    GPIO_PIN_9

#define KEY4_PORT   GPIOA
#define KEY4_PIN    GPIO_PIN_8


typedef enum
{
	KEY0,
//	KEY1,
//	KEY2,
//	KEY3,
//	KEY4,
	KEYNUM,		//按键数量
}EN_KEYNUM;		//定义按键

					

// 按键检测过程
enum
{
	KEY_PRESSWAIT=0,			//等待按键
	KEY_PRESSFIRST,				//按键按下
	KEY_COUNTTIME,				//按下计时
	KEY_RELEASEWAIT,  			//等待释放
	KEY_PELEASE					//释放消抖
};


typedef enum
{	
	KEY_WAIT,       	 		 						//等待按键
	KEY_CLICK,          								//单击确认
	KEY_CLICK_RELEASE,            						//单击释放
	KEY_LONG_PRESS,			   						 	//长按确认
	KEY_LONG_PRESS_CONTINUE,							//长按持续
	KEY_LONG_PRESS_RELEASE								//长按释放
	 
}KEY_VALUE_TYPEDEF;

typedef void (*KeyEvent_CallBack_t)(EN_KEYNUM keys,KEY_VALUE_TYPEDEF sta);


//按键消抖时间,以10ms为Tick,3=30ms
#define KEY_SCANTIME	    3		//30ms

//连续长按时间
#define	KEY_PRESSLONGTIME	200	//2s

//持续长按间隔时间
#define KEY_CONTPRESSTIME	150	//1.5秒
		
extern unsigned char registeredKeysTaskID;
		
void hal_keyInit(KeyEvent_CallBack_t pCBS);
void hal_KeyProc(void);





#endif

