/**
 ******************************************************************************
 * @file           : timer_us.c
 * @brief          : 1 MHz serbest calisan zaman sayaci (TIM2, 32 bit)
 ******************************************************************************
 */

#include "timer_us.h"
#include "main.h"

static TIM_HandleTypeDef htim2;

void timer_us_init(void)
{
    RCC_ClkInitTypeDef clkconfig;
    uint32_t pFLatency;
    uint32_t timclock;

    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

    /* APB1 bolucu 1 degilse zamanlayici saati PCLK1'in iki katidir (168 MHz'de 84 MHz) */
    if (clkconfig.APB1CLKDivider == RCC_HCLK_DIV1)
    {
        timclock = HAL_RCC_GetPCLK1Freq();
    }
    else
    {
        timclock = 2U * HAL_RCC_GetPCLK1Freq();
    }

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = (timclock / 1000000U) - 1U;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFFU;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
}
