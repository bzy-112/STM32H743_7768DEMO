#include "spi.h"
#include "stdio.h"
#include "ad7768.h"

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart1;
#define CS_0()  HAL_GPIO_WritePin(CS_GPIO_Port,CS_Pin,GPIO_PIN_RESET)
#define CS_1()  HAL_GPIO_WritePin(CS_GPIO_Port,CS_Pin,GPIO_PIN_SET)

uint16_t add7768_write_cmd(uint8_t _address, uint8_t _data)
{
	uint8_t redata[2] = {0};
	uint8_t tedata[2] = {_address,_data};
	uint8_t state;
	CS_0();
	state = HAL_SPI_Transmit(&hspi1,tedata,2,100);
	CS_1();
	CS_0();
	HAL_SPI_Receive(&hspi1,redata,2,100);
	CS_1();
	return redata[0] << 8 | redata[1];
}

void ad7768_gain_set(uint8_t chn, uint32_t gain)
{
	add7768_write_cmd(AD7768_REG_CH_GAIN_1(chn-1), (gain>>16) & 0x000000ff);
	add7768_write_cmd(AD7768_REG_CH_GAIN_2(chn-1), (gain>>8)  & 0x000000ff);
	add7768_write_cmd(AD7768_REG_CH_GAIN_3(chn-1), (gain>>0)  & 0x000000ff);
}


void ad7768_init(void)
{
//	add7768_write_cmd( 0x04,0x00);
//	add7768_write_cmd( 0x07,0x00);
//	add7768_write_cmd( 0x06,0x80);
//	add7768_write_cmd( 0x11,0xFF);
//	add7768_write_cmd( 0x12,0xFF);
//	add7768_write_cmd( 0x13,0xFF);
//	add7768_write_cmd( 0x14,0xFF);
//	HAL_Delay(100);
		uint16_t bzy = 0;
	bzy = add7768_write_cmd(AD7768_REG_CH_STANDBY,      0x00);
	printf("bzy = %x\n",bzy);
	add7768_write_cmd(AD7768_REG_CH_MODE_A,        0x0C);	//设置模式A 采样x512(512个点里取一个点采集)
	add7768_write_cmd(AD7768_REG_CH_MODE_B,        0x0D);	//设置模式B 采样x1024
	add7768_write_cmd(AD7768_REG_CH_MODE_SEL,      0x00);	//设置所有通道使用模式A		0表示模式A 1表示模式B
	add7768_write_cmd(AD7768_REG_PWR_MODE,         0x00);	//0X33	电源模式fast  MCLK分频/4（8.192Mhz）	0X00 低功耗模式 MCLK分频/32（1.024Mhz）
	add7768_write_cmd(AD7768_REG_GENERAL_CFG,      0x08);	//默认
	add7768_write_cmd(AD7768_REG_DATA_CTRL,        0x80);	//最高位为开始采样命令，先写1 再写0可软件开启转换
	add7768_write_cmd(AD7768_REG_INTERFACE_CFG,    0x00);	//DCLK无分频 = MCLK = 1.024Mhz
	//晶振32.768Mhz	DLCK = 1.024Mhz	采样率 = 1.024Mhz / 512 = 2KHz
	add7768_write_cmd(AD7768_REG_PRECHARGE_BUF_1,  0xff);	//默认
	add7768_write_cmd(AD7768_REG_PRECHARGE_BUF_2,  0xff);	//默认
	add7768_write_cmd(AD7768_REG_POS_REF_BUF,      0x00);	//默认
	add7768_write_cmd(AD7768_REG_NEG_REF_BUF,      0x00);	//默认

	for(uint8_t i=0; i<8; i++)
	{
		ad7768_gain_set(i+1, 0x555555);
	}

	uint16_t read=0;
	char buf[10]=" ";
	char reg = 0;
	for (int i = 0; i < 0x5A; i++)
	{
		read= add7768_write_cmd( reg | 0x80,0x00);
		sprintf(buf,"%x=%x",reg,read);
		printf("%s\r\n",buf);
		reg++;
	}
}


void start(void)
{
	add7768_write_cmd(AD7768_REG_DATA_CTRL, 0x00);
	HAL_Delay(30);
	add7768_write_cmd(AD7768_REG_DATA_CTRL, 0x80);
}


volatile uint8_t spi2_rx_done = 0;
	uint8_t  buf[32];
/* ================================================================
 * 纯硬件 NSS 模式数据接收 & 解析
 *
 * SPI2 配为 NSS_HARD_INPUT, DRDY → NSS 引脚.
 * HAL_SPI_Receive 自动等待 NSS↓ 然后收 32 字节.
 * 在主循环里每次调用 ad7768_read_and_print() 即可.
 * ================================================================ */
void ad7768_read_and_print(void)
{
	
	int raw;
	float    value[8];
	uint8_t  ch;
	int      ret;

	/* 关一下 SPI 清空移位寄存器, 再开时等 NSS↓ 从头收 */
//	__HAL_SPI_DISABLE(&hspi2);
//	ret = HAL_SPI_Receive(&hspi2, buf, 32, 500);
//	if (ret != HAL_OK) return;

	if (spi2_rx_done)
	{
		/* ---- 解析 8 通道 ---- */
		for (ch = 0; ch < 8; ch++) {
		
			/* buf[ch*4+0..3] = 完整 32bit 帧 (Header + 24bit ADC) */
			raw = ((uint32_t)buf[ch*4+1] << 16) |
				  ((uint32_t)buf[ch*4+2] << 8)  |
				   (uint32_t)buf[ch*4+3];

			/* 码值 -> 电压 (参考算法) */
			if ((raw & 0x00800000) == 0x00800000)
			{
				raw |= 0xFF000000;
			}else
			{
				raw &= 0x00FFFFFF;
			}
				value[ch] = (raw / 8388608.0f * 4.095000);
		}
//		printf("%6f,%6f,%6f,%6f,%6f,%6f,%6f,%6f\r\n",
//			   value[0], value[1], value[2], value[3],
//			   value[4], value[5], value[6], value[7]);
//		printf("%x,%x,%x,%x,%x,%x,%x,%x\r\n",buf[0*4],buf[1*4],buf[2*4],buf[3*4],buf[4*4],buf[5*4],buf[6*4],buf[7*4]);
	
	}

	/* ---- 打印 ---- */
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi2)
    {
        spi2_rx_done = 1;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == NSS_Pin)
	{
		if (HAL_SPI_GetState(&hspi2) == HAL_SPI_STATE_READY)
		{
			HAL_SPI_Receive_IT(&hspi2, buf, 32);
		}
	}
}
