/**
 ******************************************************************************
 * @file           : stm32f4xx_hal_timebase_tim.c
 * @brief          : HAL zaman tabani TIM7 uzerinden (SysTick FreeRTOS'a birakildi)
 ******************************************************************************
 */

#include "main.h"

TIM_HandleTypeDef htim7;

/* HAL_IncTick'i surmek icin TIM7'yi 1 kHz guncelleme kesmesi uretecek sekilde kurar.
 * HAL_Init ve HAL_RCC_ClockConfig tarafindan cagrilir; her saat degisiminde yeniden hesaplar. */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    RCC_ClkInitTypeDef clkconfig;
    uint32_t uwTimclock;
    uint32_t pFLatency;
    HAL_StatusTypeDef status;

    __HAL_RCC_TIM7_CLK_ENABLE();

    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

    /* APB1 bolucu 1 degilse zamanlayici saati PCLK1'in iki katidir */
    if (clkconfig.APB1CLKDivider == RCC_HCLK_DIV1)
    {
        uwTimclock = HAL_RCC_GetPCLK1Freq();
    }
    else
    {
        uwTimclock = 2U * HAL_RCC_GetPCLK1Freq();
    }

    /* 1 MHz sayac, 1000 sayimda tasma -> 1 ms */
    htim7.Instance = TIM7;
    htim7.Init.Prescaler = (uwTimclock / 1000000U) - 1U;
    htim7.Init.Period = (1000000U / 1000U) - 1U;
    htim7.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    status = HAL_TIM_Base_Init(&htim7);
    if (status == HAL_OK)
    {
        status = HAL_TIM_Base_Start_IT(&htim7);
        if (status == HAL_OK)
        {
            if (TickPriority < (1UL << __NVIC_PRIO_BITS))
            {
                HAL_NVIC_SetPriority(TIM7_IRQn, TickPriority, 0U);
                HAL_NVIC_EnableIRQ(TIM7_IRQn);
                uwTickPrio = TickPriority;
            }
            else
            {
                status = HAL_ERROR;
            }
        }
    }

    return status;
}

void HAL_SuspendTick(void)
{
    __HAL_TIM_DISABLE_IT(&htim7, TIM_IT_UPDATE);
}

void HAL_ResumeTick(void)
{
    __HAL_TIM_ENABLE_IT(&htim7, TIM_IT_UPDATE);
}
