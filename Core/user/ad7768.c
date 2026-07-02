#include "spi.h"
#include "stdio.h"
#include "ad7768.h"
#include "hal_key.h"
//#include "ad7768_data_gpio.h"

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim6;
#define CS_0() HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET)
#define CS_1() HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET)

static uint16_t ad7768_write_reg(uint8_t reg, uint8_t val) {
  uint8_t tx[2] = {(uint8_t)(reg & 0x7F), val};
  CS_0();
  HAL_SPI_Transmit(&hspi1, tx, 2, 100);
  CS_1();
  return 0;
}

static uint16_t ad7768_read_reg(uint8_t reg) {
  uint8_t tx1[2] = {(uint8_t)(0x80 | (reg & 0x7F)), 0x00};
  uint8_t rx[2] = {0};

  CS_0();
  HAL_SPI_Transmit(&hspi1, tx1, 2, 100);
  CS_1();

  uint8_t tx2[2] = {0x00, 0x00};
  CS_0();
  HAL_SPI_TransmitReceive(&hspi1, tx2, rx, 2, 100);
  CS_1();

  return ((uint16_t)rx[0] << 8) | rx[1];
}

uint16_t add7768_write_cmd(uint8_t _address, uint8_t _data) {
  uint8_t redata[2] = {0};
  uint8_t tedata[2] = {_address, _data};
  uint8_t state;
  CS_0();
  state = HAL_SPI_Transmit(&hspi1, tedata, 2, 100);
  CS_1();
  CS_0();
  HAL_SPI_Receive(&hspi1, redata, 2, 100);
  CS_1();
  return redata[0] << 8 | redata[1];
}

void ad7768_gain_set(uint8_t chn, uint32_t gain) {
  add7768_write_cmd(AD7768_REG_CH_GAIN_1(chn - 1), (gain >> 16) & 0x000000ff);
  add7768_write_cmd(AD7768_REG_CH_GAIN_2(chn - 1), (gain >> 8) & 0x000000ff);
  add7768_write_cmd(AD7768_REG_CH_GAIN_3(chn - 1), (gain >> 0) & 0x000000ff);
}

typedef enum {
  NOMultiple,         // 不放大	000
  TWOMultiple,        // 两倍		001
  FOURultiple,        // 四倍		010
  EightMultiple,      // 八倍		011
  sixteenMultiple,    // 16		100
  thirty_twoMultiple, // 32		101
  Sixty_fourMultiple, // 64		110
  MAXMultiple         // 120		111
} Mul_type;
void start(void) {
  add7768_write_cmd(AD7768_REG_DATA_CTRL, 0x00);
  HAL_Delay(30);
  add7768_write_cmd(AD7768_REG_DATA_CTRL, 0x80);
}

void set_Multiple(Mul_type Multiple) {
  //	printf("hellow\n");
  HAL_GPIO_WritePin(A2_GPIO_Port, A2_Pin, Multiple >> 2 & 0x01);
  HAL_GPIO_WritePin(A1_GPIO_Port, A1_Pin, Multiple >> 1 & 0x01);
  HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, Multiple >> 0 & 0x01);
}
uint8_t Multiple = 0;
Mul_type bzy = NOMultiple;
void fuwei(void) {
  ad7768_write_reg(0x06, 0x03);
  ad7768_write_reg(0x06, 0x02);
  ad7768_write_reg(AD7768_REG_CH_STANDBY, 0x00);
  ad7768_write_reg(AD7768_REG_CH_MODE_A,
                   0x0C); // 设置模式A 采样x512(512个点里取一个点采集)
  ad7768_write_reg(AD7768_REG_CH_MODE_B, 0x0D); // 设置模式B 采样x1024
  ad7768_write_reg(
      AD7768_REG_CH_MODE_SEL,
      0x00); // 设置所有通道使用模式A		0表示模式A 1表示模式B
  ad7768_write_reg(AD7768_REG_PWR_MODE, 0x33);    // 0X33	电源模式fast
                                                  // MCLK分频/4（8.192Mhz）
                                                  // 0X00 低功耗模式
                                                  // MCLK分频/32（1.024Mhz）
  ad7768_write_reg(AD7768_REG_GENERAL_CFG, 0x08); // 默认
  ad7768_write_reg(AD7768_REG_DATA_CTRL,
                   0x80); // 最高位为开始采样命令，先写1 再写0可软件开启转换
  ad7768_write_reg(AD7768_REG_INTERFACE_CFG,
                   0x00); // DCLK无分频 = MCLK = 1.024Mhz
  // 晶振32.768Mhz	DLCK = 1.024Mhz	采样率 = 1.024Mhz / 512 = 2KHz
  ad7768_write_reg(AD7768_REG_PRECHARGE_BUF_1, 0x00); // 默认
  ad7768_write_reg(AD7768_REG_PRECHARGE_BUF_2, 0x00); // 默认
  ad7768_write_reg(AD7768_REG_POS_REF_BUF, 0xff);     // 默认
  ad7768_write_reg(AD7768_REG_NEG_REF_BUF, 0xff);     // 默认
}

void Key_CallBack_t(EN_KEYNUM keys, KEY_VALUE_TYPEDEF sta) {
  if (keys == KEY0) {
    if (sta == KEY_CLICK) {
      uint16_t read = 0;
      if (bzy == MAXMultiple)
        bzy = NOMultiple;
      // fuwei();
      set_Multiple(bzy);
      // start();
      bzy++;
    }
  }
}
volatile uint8_t spi2_rx_done = 0;
uint8_t buf[32];
volatile uint8_t buf_b[32];
uint32_t data_spi[8];

void ad7768_init(void) {
  ad7768_write_reg(AD7768_REG_CH_STANDBY, 0x00);
  ad7768_write_reg(AD7768_REG_CH_MODE_A,
                   0x02); // 设置模式A 采样x512(512个点里取一个点采集)
  ad7768_write_reg(AD7768_REG_CH_MODE_B, 0x0D); // 设置模式B 采样x1024
  ad7768_write_reg(
      AD7768_REG_CH_MODE_SEL,
      0x00); // 设置所有通道使用模式A		0表示模式A 1表示模式B
  ad7768_write_reg(AD7768_REG_PWR_MODE, 0x00);    // 0X33	电源模式fast
                                                  // MCLK分频/4（8.192Mhz）
                                                  // 0X00 低功耗模式
                                                  // MCLK分频/32（1.024Mhz）
  ad7768_write_reg(AD7768_REG_GENERAL_CFG, 0x08); // 默认
  ad7768_write_reg(AD7768_REG_DATA_CTRL,
                   0x80); // 最高位为开始采样命令，先写1 再写0可软件开启转换
  ad7768_write_reg(AD7768_REG_INTERFACE_CFG,
                   0x00); // DCLK无分频 = MCLK = 1.024Mhz
  // 晶振32.768Mhz	DLCK = 1.024Mhz	采样率 = 1.024Mhz / 512 = 2KHz
  ad7768_write_reg(AD7768_REG_PRECHARGE_BUF_1, 0x00); // 默认
  ad7768_write_reg(AD7768_REG_PRECHARGE_BUF_2, 0x00); // 默认
  ad7768_write_reg(AD7768_REG_POS_REF_BUF, 0xff);     // 默认
  ad7768_write_reg(AD7768_REG_NEG_REF_BUF, 0xff);     // 默认

  for (uint8_t i = 0; i < 8; i++) {
    ad7768_gain_set(i + 1, 0x555555);
  }

  uint16_t read = 0;
  char buf[10] = " ";
  char reg = 0;
  for (int i = 0; i < 0x5A; i++) {
    read = ad7768_read_reg(reg);
    sprintf(buf, "%x=%x\n\0", reg, read);
    printf("%s\r\n", buf);
    reg++;
  }
  set_Multiple(bzy);
  HAL_TIM_Base_Start_IT(&htim6);
  hal_keyInit(Key_CallBack_t);
//      HAL_SPI_Receive_IT(&hspi2, buf, 32);
}
volatile uint16_t count[2] = {0};
/* ================================================================
 * 纯硬件 NSS 模式数据接收 & 解析
 *
 * SPI2 配为 NSS_HARD_INPUT, DRDY → NSS 引脚.
 * HAL_SPI_Receive 自动等待 NSS↓ 然后收 32 字节.
 * 在主循环里每次调用 ad7768_read_and_print() 即可.
 * ================================================================ */
void ad7768_read_and_print(void) {
  float value[8];
  uint8_t ch;
  int ret;
	uint8_t left_o = 0;
  /*
          while(1)
          {
                  AD7768_ReadFrame(buf,1000);
                  for (ch = 0; ch < 8; ch++)
                  {
                          raw = ((uint32_t)buf[ch*4+1] << 16) |
                                    ((uint32_t)buf[ch*4+2] << 8)  |
                                     (uint32_t)buf[ch*4+3];
                          // 码值 -> 电压 (参考算法)
                          if ((raw & 0x00800000) == 0x00800000)
                          {
                                  raw |= 0xFF000000;
                          }else
                          {
                                  raw &= 0x00FFFFFF;
                          }
                          value[ch] = (float)raw * 4.096f / 8388608.0f;
                  }
                                  printf("%6f,%6f,%6f,%6f,%6f,%6f,%6f,%6f\r\n",
                             value[0], value[1], value[2], value[3],
                             value[4], value[5], value[6], value[7]);

          }
  */
  if (spi2_rx_done) {
	spi2_rx_done = 0;
	memcpy(buf_b, buf, sizeof(buf));
	if (((buf_b[0]) & 0x07) == 1) // 检查通道号
	{
		left_o = 1;
	}
    for (ch = 0; ch < 8; ch++) {
      int raw;
//	printf("count: io = %d, spi = %d\n",count[1],count[0]);
      /* buf[ch*4+0..3] = 完整 32bit 帧 (Header + 24bit ADC) */
      raw = ((uint32_t)buf_b[ch * 4] << 24) |
            ((uint32_t)buf_b[ch * 4 + 1] << 16) |
            ((uint32_t)buf_b[ch * 4 + 2] << 8) | (uint32_t)buf_b[ch * 4 + 3];
		raw >>= left_o;
		if ((raw >> 24 & 0x07) != ch) // 检查通道号
		{
			printf("head->ch0:%x ,ch1:%x ,ch2:%x ,ch3:%x ,ch4:%x ,ch5:%x ,ch6:%x,ch7:%x\n",buf_b[0],buf_b[4],buf_b[8],buf_b[12],buf_b[16],buf_b[20],buf_b[24],buf_b[28]);
			printf("err\n");
			return;
		}
      if ((raw & 0x00800000) == 0x00800000) // 负数，符号扩展
      {
        raw |= 0xFF000000;
      } else {
        raw &= 0x00FFFFFF;
      }
      value[ch] = (float)(raw) / 8388608.0f * 4.096f;
    }
//    printf("%6f,%6f,%6f,%6f,%6f,%6f,%6f,%6f\r\n", value[0], value[1], value[2],value[3], value[4], value[5], value[6], value[7]);
    //		for(int i = 0;i < 8; i++)
    //		{
    //			printf("%x,%x,%x,%x,",buf[i*4],buf[i*4+1],buf[i*4+2],buf[i*4+3]);
    //		}
    //		printf("\n");
  }

  /* ---- 打印 ---- */
}
volatile uint16_t rececount_io = 0;
volatile uint16_t rececount_spi = 0;
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi == &hspi2) {
    spi2_rx_done = 1;
//      HAL_SPI_Receive_IT(&hspi2, buf, 32);
	  rececount_spi++;
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == NSS_Pin) {
    if (HAL_SPI_GetState(&hspi2) == HAL_SPI_STATE_READY) {
      HAL_SPI_Receive_IT(&hspi2, buf, 32);
    }
	rececount_io++;
  }
}
