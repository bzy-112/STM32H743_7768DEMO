#include "hal_key.h"
#include "string.h"

KeyEvent_CallBack_t KeyScanCBS;

								
unsigned char KeyState[KEYNUM];			//按键状态
unsigned short KeyScanTime[KEYNUM];		//去抖延时
unsigned short KeyPressLongTimer[KEYNUM];	//长按延时
unsigned short KeyContPressTimer[KEYNUM];	//连续长按延时	

static unsigned char hal_getKEY0Sta(void)
{
	return (GPIO_STATE(KEY0_PORT, KEY0_PIN));
}

static unsigned char hal_getKEY1Sta(void)
{
	return (GPIO_STATE(KEY1_PORT, KEY1_PIN));		
}
 
static unsigned char hal_getKEY2Sta(void)
{
	return (GPIO_STATE(KEY2_PORT, KEY2_PIN));		
}
 
static unsigned char hal_getKEY3Sta(void)
{
	return (GPIO_STATE(KEY3_PORT, KEY3_PIN));		
}
 
static unsigned char hal_getKEY4Sta(void)
{
	return (GPIO_STATE(KEY4_PORT, KEY4_PIN));		
}


static unsigned char (*getKeysVal[KEYNUM])() = { 
                        hal_getKEY0Sta,
//                        hal_getKEY1Sta,
//                        hal_getKEY2Sta,
//                        hal_getKEY3Sta,
//	                    hal_getKEY4Sta
                    };



static void hal_KeyScanCBSRegister(KeyEvent_CallBack_t pCBS)
{
	if(KeyScanCBS == 0)
	{
			KeyScanCBS = pCBS;
	}
}	

void hal_keyInit(KeyEvent_CallBack_t pCBS)
{
	unsigned char i;
	hal_KeyScanCBSRegister(pCBS);
	for(i=0; i<KEYNUM; i++)
	{
		KeyState[i] = KEY_PRESSWAIT;
		KeyScanTime[i] = KEY_SCANTIME;
		KeyPressLongTimer[i] = KEY_PRESSLONGTIME;
		KeyContPressTimer[i] = KEY_CONTPRESSTIME;
	}
	
}

 

void hal_KeyProc(void)
{
	unsigned char i,KeyLineTemp[KEYNUM],keys;
	unsigned char state;
	memset(KeyLineTemp, 0,KEYNUM);
	for(i = 0; i < KEYNUM; i ++)
	{
		KeyLineTemp[i] = (*(getKeysVal[i]))();	//获取所有按键状态
	}
	for(i=0; i<KEYNUM; i++)					//遍历按键
	{	
		keys = 0xff; 
		state = 0;
		switch(KeyState[i])					//当前按键状态
		{
			case KEY_PRESSWAIT:
				if(!KeyLineTemp[i])
				{
					KeyState[i] = KEY_PRESSFIRST;	
				}
			break;
			case KEY_PRESSFIRST:
				if(!KeyLineTemp[i])
				{
					if(!(--KeyScanTime[i]))
					{
						KeyScanTime[i] = KEY_SCANTIME;
						KeyState[i] = KEY_COUNTTIME;
						keys = i;										//记录按键ID号
				 		state = KEY_CLICK;								//按键单击
						 
					}
				}else
				{
					KeyScanTime[i] = KEY_SCANTIME;
					KeyState[i] = KEY_PRESSWAIT;
				}
			break;
			case KEY_COUNTTIME:
				if(!KeyLineTemp[i])
				{	
					if(!(--KeyPressLongTimer[i]))
					{
						KeyPressLongTimer[i] = KEY_PRESSLONGTIME;
						KeyState[i] = KEY_RELEASEWAIT;
						
						keys = i;										//记录按键ID号
				  	state = KEY_LONG_PRESS;							//长按确认
							
					}
				}else
				{
					KeyPressLongTimer[i] = KEY_PRESSLONGTIME;
					KeyState[i] = KEY_PELEASE;
					keys = i;										//记录按键ID号
				 	state = KEY_CLICK_RELEASE;						//单击释放
				
				}
			break;
			case KEY_RELEASEWAIT:
				if(!KeyLineTemp[i])
				{
					KeyState[i] = KEY_PELEASE;
					KeyContPressTimer[i] = KEY_CONTPRESSTIME;
					keys = i;								//记录按键ID号
			 	state = KEY_LONG_PRESS_RELEASE; 		//长按释放
				}else
				{
					if(!--KeyContPressTimer[i])
					{
						KeyContPressTimer[i] = KEY_CONTPRESSTIME;
						keys = i;							//持续长按
				  	state = KEY_LONG_PRESS_CONTINUE;
						 
					}
				}
			break;
			case KEY_PELEASE:
				if(!KeyLineTemp[i])
				{
					KeyState[i] = KEY_PRESSWAIT;
				}
			break;			
		}
		
		if(keys != 0xff)
		{
			if(KeyScanCBS)
			{
				KeyScanCBS((EN_KEYNUM)keys,(KEY_VALUE_TYPEDEF)state);
			}
		}
	}
}

