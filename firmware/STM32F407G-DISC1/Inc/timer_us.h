/**
 ******************************************************************************
 * @file           : timer_us.h
 * @brief          : 1 MHz serbest calisan zaman sayaci (TIM2, 32 bit)
 ******************************************************************************
 */

#ifndef TIMER_US_H
#define TIMER_US_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/*
 * TIM2'yi 1 us cozunurlukle, 0xFFFFFFFF'de sarilan serbest sayac olarak baslatir.
 * SystemClock_Config'ten sonra cagrilmali. Kesme kullanmaz.
 */
void timer_us_init(void);

/* Mikrosaniye cinsinden anlik zaman. ISR icinden de cagrilabilir (tek register okumasi). */
static inline uint32_t timer_us(void)
{
    return TIM2->CNT;
}

/*
 * Iki zaman damgasi arasindaki fark (us). Sayac ~71.6 dakikada (2^32 us) sarilir;
 * isaretsiz cikarma mod 2^32 calistigi icin tek bir sarilmayi dogru hesaplar.
 */
static inline uint32_t timer_us_elapsed(uint32_t start, uint32_t end)
{
    return end - start;
}

#ifdef __cplusplus
}
#endif

#endif /* TIMER_US_H */
