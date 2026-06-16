/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

#include <stdio.h>

int fputc(int ch, FILE *f)
{
    uint8_t data = (uint8_t)ch;
    HAL_UART_Transmit(&huart1, &data, 1, 1000);
    return ch;
}

#define RECE_SIZE		200					//最大不超过0x1000 / 4

// Memory0（rx1_buf_a）满的回调
void My_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma);
// Memory1（rx1_buf_b）满的回调
void My_DMA_XferM1CpltCallback(DMA_HandleTypeDef *hdma);
// DMA错误回调
void My_DMA_ErrorCallback(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef UARTEx_MultiBuffer_ReceiveToIdle(void);

__attribute__((section(".dmauart1_buffer"), aligned(32))) uint32_t rx1_buf_a[RECE_SIZE];
__attribute__((section(".dmauart1_buffer"), aligned(32))) uint32_t rx1_buf_b[RECE_SIZE];

volatile uint8_t current_buffer = 0;//缓冲区标志
volatile uint8_t actual_rx_len = 0;//接受数据长度
volatile uint8_t rx_complete_flag = 0;//缓冲区满标志
volatile uint8_t idle_triggered  = 0;//空闲中断标志位


__attribute__((section(".dmauart1_buffer"), aligned(32))) uint8_t tx_buf_a[RECE_SIZE];
__attribute__((section(".dmauart1_buffer"), aligned(32))) uint8_t tx_buf_b[RECE_SIZE];

volatile uint8_t tx_current_buf = 0;    // 当前 DMA 正在发送哪一块（0 -> A，1 -> B）
volatile uint8_t tx_ready_flag = 0;     // CPU 已经填好下一块
volatile uint8_t tx_idle_flag  = 1;     // 初始空闲

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */
	huart1.ReceptionType = HAL_UART_RECEPTION_TOIDLE;
	huart1.RxEventType = HAL_UART_RXEVENT_IDLE;
	huart1.RxXferSize = RECE_SIZE;
  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA1_Stream1;
    hdma_usart1_rx.Init.Request = DMA_REQUEST_USART1_RX;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA1_Stream2;
    hdma_usart1_tx.Init.Request = DMA_REQUEST_USART1_TX;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart1_tx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */
	
  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Stream3;
    hdma_usart2_rx.Init.Request = DMA_REQUEST_USART2_RX;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Stream4;
    hdma_usart2_tx.Init.Request = DMA_REQUEST_USART2_TX;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart2_tx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 2, 3);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5|GPIO_PIN_6);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void UART_Tx_DMA_Cplt(DMA_HandleTypeDef *hdma)
{
    tx_current_buf = 1;     // 刚发送完 A，切换到 B
    tx_idle_flag  = 1;      // 这块缓冲空出来了
}

void UART_Tx_DMA_M1Cplt(DMA_HandleTypeDef *hdma)
{
    tx_current_buf = 0;     // 刚发送完 B，切换回 A
    tx_idle_flag  = 1;
}

void UART_Tx_DMA_Error(DMA_HandleTypeDef *hdma)
{
    // 记录错误，必要时复位 DMA
}


// Memory0（rx1_buf_a）满的回调
void My_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma)
{
 if(hdma->Instance == DMA1_Stream1 && !idle_triggered)
 {
   current_buffer = 1;  // 切换到Memory1（rx1_buf_b）
   actual_rx_len = RECE_SIZE;
   rx_complete_flag = 1; // 标记缓冲区满，供主函数处理rx1_buf_a
 }
}

// Memory1（rx1_buf_b）满的回调
void My_DMA_XferM1CpltCallback(DMA_HandleTypeDef *hdma)
{
 if(hdma->Instance == DMA1_Stream1 && !idle_triggered)
 {
   current_buffer = 0;  // 切换到Memory0（rx1_buf_a）
   actual_rx_len = RECE_SIZE;
   rx_complete_flag = 1; // 标记缓冲区满，供主函数处理rx1_buf_b
 }
}

// DMA错误回调
void My_DMA_ErrorCallback(DMA_HandleTypeDef *hdma)
{
 if(hdma->Instance == DMA1_Stream1)
 {
   // 清除错误，重启双缓冲区DMA
   HAL_DMA_Abort(&hdma_usart1_rx);
   HAL_DMAEx_MultiBufferStart_IT(&hdma_usart1_rx,
                                (uint32_t)&USART1->RDR,
                                (uint32_t)rx1_buf_a,
                                (uint32_t)rx1_buf_b,
                                RECE_SIZE);
 }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart->Instance == USART1)
	{
		if(Size != RECE_SIZE )
		{
			idle_triggered = 1;
			actual_rx_len = Size;
		}
	}
}

void Send_RxData(void)
{
	if(rx_complete_flag && !idle_triggered)
	{
		if(actual_rx_len == RECE_SIZE)
		{
			// 缓冲区满：处理对应缓冲区的数据
			if(current_buffer == 1) // 刚填满buf_a，切换到buf_b
			{
				printf("buf_a满\r\n");
				HAL_UART_Transmit(&huart1,rx1_buf_a,RECE_SIZE, 0xFFFF);
				printf("\r\n");
			}
			else // 刚填满buf_b，切换到buf_a
			{
				printf("buf_b满\r\n");
				HAL_UART_Transmit(&huart1,rx1_buf_b,RECE_SIZE, 0xFFFF);
				printf("\r\n");
			}
			// 清空标志，准备下一次接收
			rx_complete_flag = 0;
		}
	}
	if(idle_triggered && !rx_complete_flag)
	{
		// IDLE触发：处理当前活跃缓冲区的剩余数据
		idle_triggered = 0; // 重置IDLE标记
		if(actual_rx_len > 0){
			if(current_buffer == 0) // 活跃缓冲区是buf_a
			{
				printf("IDLE触发 buf_a:\r\n");
				HAL_UART_Transmit(&huart1,rx1_buf_a,actual_rx_len, 0xFFFF);
				printf("\r\n");
			}
			else // 活跃缓冲区是buf_b
			{
				printf("IDLE触发 buf_b:\r\n");
				HAL_UART_Transmit(&huart1,rx1_buf_b,actual_rx_len, 0xFFFF);
				printf("\r\n");
			}
		}
	}
}

HAL_StatusTypeDef UARTEx_MultiBuffer_ReceiveToIdle(void)
{
	HAL_StatusTypeDef status;
  if (huart1.RxState == HAL_UART_STATE_READY)
  {

    	huart1.ReceptionType = HAL_UART_RECEPTION_TOIDLE;
    	huart1.RxEventType = HAL_UART_RXEVENT_IDLE;
		huart1.RxXferSize = RECE_SIZE;

		huart1.ErrorCode = HAL_UART_ERROR_NONE;
		huart1.RxState = HAL_UART_STATE_BUSY_RX;
		HAL_DMA_RegisterCallback(&hdma_usart1_rx,HAL_DMA_XFER_CPLT_CB_ID, My_DMA_XferCpltCallback);
		// Memory1（buf_b）满触发的回调
		HAL_DMA_RegisterCallback(&hdma_usart1_rx, HAL_DMA_XFER_M1CPLT_CB_ID, My_DMA_XferM1CpltCallback);
		// DMA错误回调
		HAL_DMA_RegisterCallback(&hdma_usart1_rx, HAL_DMA_XFER_ERROR_CB_ID, My_DMA_ErrorCallback);
		status =  HAL_DMAEx_MultiBufferStart_IT(&hdma_usart1_rx,(uint32_t)&(USART1->RDR),(uint32_t)rx1_buf_a,(uint32_t)rx1_buf_b,RECE_SIZE);
	    if (huart1.ReceptionType == HAL_UART_RECEPTION_TOIDLE)
	    {
			__HAL_UART_CLEAR_IDLEFLAG(&huart1);
			ATOMIC_SET_BIT(huart1.Instance->CR1, USART_CR1_IDLEIE);
			ATOMIC_SET_BIT(huart1.Instance->CR3, USART_CR3_DMAR);
	    }
	    else
	    {
	      status = HAL_ERROR;
	    }
	
	    return status;
	  }
	  else
	  {
	    return HAL_BUSY;
	  }
	
}


/* USER CODE END 1 */
