/**
 ******************************************************************************
 * @file           : stm32f4xx_it.c
 * @brief          : Kesme servis rutinleri
 ******************************************************************************
 */

#include "main.h"
#include "stm32f4xx_it.h"

void NMI_Handler(void)
{
    for (;;)
    {
    }
}

void HardFault_Handler(void)
{
    for (;;)
    {
    }
}

void MemManage_Handler(void)
{
    for (;;)
    {
    }
}

void BusFault_Handler(void)
{
    for (;;)
    {
    }
}

void UsageFault_Handler(void)
{
    for (;;)
    {
    }
}

/*
 * SVC_Handler ve PendSV_Handler FreeRTOS portundan gelir (FreeRTOSConfig.h eslemesi),
 * SysTick_Handler ise cmsis_os2.c'den. HAL zaman tabani TIM7'dedir.
 */
void TIM7_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim7);
}

/* Buton kesmesi; ISR mantigi (t0, 30 ms filtre, buttonQ) HAL_GPIO_EXTI_Callback'te eklenecek */
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(B1_Pin);
}

void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}
