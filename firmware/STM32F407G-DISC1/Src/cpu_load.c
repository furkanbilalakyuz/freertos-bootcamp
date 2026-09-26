/**
 ******************************************************************************
 * @file           : cpu_load.c
 * @brief          : S4/S5 senaryolari icin kalibre edilmis CPU yuku
 ******************************************************************************
 */

#include "cpu_load.h"
#include "main.h"
#include "timer_us.h"

/* Kalibrasyonda kosulan iterasyon; 168 MHz'de birkac ms surer */
#define CAL_ITERS   100000U

uint32_t calibrated_work_iters_per_us_q16;
volatile uint32_t calibrated_work_sink;

/*
 * Her iterasyon bir onceki sonuca bagli (LCG), bos asm engeli x'i "kullanilmis"
 * sayar; derleyici donguyu kapali forma indiremez ya da silemez.
 */
static uint32_t work_loop(uint32_t iters, uint32_t x)
{
    for (uint32_t i = 0U; i < iters; i++)
    {
        x = (x * 1664525U) + 1013904223U;
        __asm__ volatile("" : "+r"(x));
    }
    return x;
}

void calibrated_work_calibrate(void)
{
    uint32_t primask = __get_PRIMASK();
    uint32_t start;
    uint32_t elapsed;
    uint32_t x;

    __disable_irq();
    start = timer_us();
    x = work_loop(CAL_ITERS, start);
    elapsed = timer_us_elapsed(start, timer_us());
    __set_PRIMASK(primask);

    calibrated_work_sink = x;

    if (elapsed == 0U)
    {
        Error_Handler();
    }
    calibrated_work_iters_per_us_q16 = (uint32_t)(((uint64_t)CAL_ITERS << 16) / elapsed);
}

uint32_t calibrated_work(uint32_t us)
{
    uint32_t iters = (uint32_t)(((uint64_t)us * calibrated_work_iters_per_us_q16) >> 16);
    uint32_t x = work_loop(iters, calibrated_work_sink);

    calibrated_work_sink = x;
    return x;
}
