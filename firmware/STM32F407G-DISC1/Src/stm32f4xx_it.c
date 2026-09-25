/**
 ******************************************************************************
 * @file           : stm32f4xx_it.c
 * @brief          : Kesme servis rutinleri
 ******************************************************************************
 */

#include "main.h"
#include "stm32f4xx_it.h"
#include "app_freertos.h"
#include "timer_us.h"

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

/*
 * Buton kesmesi. t0 ISR'in ilk komutunda alinir; HAL_GPIO_EXTI_IRQHandler/callback
 * zincirinin getirecegi gecikme olcume girmesin diye HAL dagitimi kullanilmaz.
 */
void EXTI0_IRQHandler(void)
{
    uint32_t t0 = timer_us();

    if (__HAL_GPIO_EXTI_GET_IT(B1_Pin) != 0U)
    {
        __HAL_GPIO_EXTI_CLEAR_IT(B1_Pin);
        button_isr(t0);
    }
}

void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}
