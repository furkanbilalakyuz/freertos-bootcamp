/**
 ******************************************************************************
 * @file           : stm32f4xx_hal_msp.c
 * @brief          : HAL MSP (dusuk seviye cevre birimi) init/deinit
 ******************************************************************************
 */

#include "main.h"

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* FreeRTOS Cortex-M4 portu tum bitlerin preemption onceligi olmasini ister */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin = USART2_TX_Pin | USART2_RX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(USART2_GPIO_Port, &GPIO_InitStruct);

        /* TC callback (t4 olcumu) icin kesme tabanli gonderim kullanilacak */
        HAL_NVIC_SetPriority(USART2_IRQn, APP_IRQ_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_DISABLE();
        HAL_GPIO_DeInit(USART2_GPIO_Port, USART2_TX_Pin | USART2_RX_Pin);
        HAL_NVIC_DisableIRQ(USART2_IRQn);
    }
}
