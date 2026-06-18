#include "spi.h"
#include "stdio.h"
#include "ad7768.h"
extern SPI_HandleTypeDef hspi1;
#define CS_0()										HAL_GPIO_WritePin(CS_GPIO_Port,CS_Pin,GPIO_PIN_RESET)
#define CS_1()										HAL_GPIO_WritePin(CS_GPIO_Port,CS_Pin,GPIO_PIN_SET)

uint16_t add7768_write_cmd(uint8_t _address, uint8_t _data)
{
	uint8_t redata[2] = {0};
	uint8_t tedata[2] = {_address,_data};
	uint8_t state;
	CS_0();
	HAL_SPI_TransmitReceive(&hspi1,tedata,redata,2,100);
	CS_1();
	return redata[1];
}

void ad7768_gain_set(uint8_t chn, uint32_t gain)
{
	add7768_write_cmd(AD7768_REG_CH_GAIN_1(chn-1), (gain>>16) & 0x000000ff);	//MSB
	add7768_write_cmd(AD7768_REG_CH_GAIN_2(chn-1), (gain>>8) & 0x000000ff);		//Mid
	add7768_write_cmd(AD7768_REG_CH_GAIN_3(chn-1), (gain>>0) & 0x000000ff);		//LSB
}


void ad7768_init(void)
{
//	AD7768_SendRecive_REG( 0x04,0x00);	//最高功耗
//	AD7768_SendRecive_REG( 0x07,0x00); // DCLK=fMOD/8=250kHz for bit-bang
//	AD7768_SendRecive_REG( 0x06,0x80);
//	
//	AD7768_SendRecive_REG( 0x11,0xFF);
//	AD7768_SendRecive_REG( 0x12,0xFF);
//	AD7768_SendRecive_REG( 0x13,0xFF);
//	AD7768_SendRecive_REG( 0x14,0xFF);
	
	add7768_write_cmd(AD7768_REG_CH_STANDBY,			0x00);		//enable所有通道
	add7768_write_cmd(AD7768_REG_CH_MODE_A,				0x0D);		//默认A Sinc滤波器 x？？？采样率设置 ，请查阅手册P75
	
	add7768_write_cmd(AD7768_REG_CH_MODE_B,				0x0D);		//默认B Sinc滤波器 x？？？采样率设置
	add7768_write_cmd(AD7768_REG_CH_MODE_SEL,			0x00);		//默认所有通道选择A
	/*POWER MODE SELECT REGISTER*/	
	add7768_write_cmd(AD7768_REG_PWR_MODE,				0x00);		//bit7			|SLEEP_MODE		：0 Normal operation. 	1 Sleep mode.	
																													//bit[5:4]	|POWER_MODE		：00 Eco mode.  10 Median mode.  11 Fast mode.																													//bit[1:0] 	|MCLK_DIV			：00 MCLK/32:		10 MCLK/8:		11 MCLK/4:
	/*GENERAL DEVICE CONFIGURATION REGISTER*/	
	add7768_write_cmd(AD7768_REG_GENERAL_CFG,			0x22);		//bit5 			|RETIME_EN		：0 Disabled		1 Enable SYNC_OUT signal from MCLK	 
																													//blt[1:0]	|VCM_VSEL			：00(AVDD1 - AVSS)/2 V.		01 1.65 V.		10 2.5V		11 2.14V  使用VCM时必须开启通道0
	/*DATA CONTROL: SOFT RESET, SYNC, AND SINGLE-SHOT CONTROL REGISTER*/	
	add7768_write_cmd(AD7768_REG_DATA_CTRL,				0x80);		//bit7			|SPI_SYNC			：0 SPI_SYNC low. 	1 SPI_SYNC high （只有1个设备默认高）
																													//bit4  		|SINGLE_SHOT	：0 Disabled.		1 Enabled.（不开启）
	/*INTERFACE CONFIGURATION REGISTER*/
	add7768_write_cmd(AD7768_REG_INTERFACE_CFG,		0x00);	  //bit[3:2]	|CRC_SELECT		：00 No CRC
	
	/*BUFFER ENABLE REGISTER 0 - 3*/
	add7768_write_cmd(AD7768_REG_PRECHARGE_BUF_1,		0xff);	//默认通道 0-3开启缓冲	
	/*BUFFER ENABLE REGISTER 4 - 7*/
	add7768_write_cmd(AD7768_REG_PRECHARGE_BUF_2,		0xff);	//默认通道 4-7开启缓冲	
	/*0-7负极参考缓冲*/
	add7768_write_cmd(AD7768_REG_POS_REF_BUF,				0x00);	//负极缓冲off		
	/*0-7正极参考缓冲*/
	add7768_write_cmd(AD7768_REG_NEG_REF_BUF,				0x00);	//正极缓冲off	
	
	
	for(uint8_t i=0; i<8; i++)
	{
		ad7768_gain_set(i+1, 0x555555);		//配置输出增益
	}
		
	//实际设置结果
	//MCLK = 4.2Mhz （1/4分频，实际输入晶振32.768M）
	//DCLK = 4.2Mhz
	//采集输入电源模式：FAST  单线D0只能采集8Mhz

	uint16_t read=0;
	char buf[10]=" ";
	for (int i = 0; i < 0x5A; i++)
	{
		read= add7768_write_cmd( i | 0x80,0x00);
		sprintf(buf,"%x=%x",i,read);
		printf("%s\r\n",buf);
	}
	
}