/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Donanim baslatma katmani: pin tanimlari ve init prototipleri
 ******************************************************************************
 */

#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* Kullanici butonu B1: PA0, harici pull-down, basildiginda HIGH */
#define B1_Pin              GPIO_PIN_0
#define B1_GPIO_Port        GPIOA
#define B1_EXTI_IRQn        EXTI0_IRQn

/* LED'ler: PD12 yesil, PD13 turuncu, PD14 kirmizi, PD15 mavi */
#define LED_GREEN_Pin       GPIO_PIN_12
#define LED_ORANGE_Pin      GPIO_PIN_13
#define LED_RED_Pin         GPIO_PIN_14
#define LED_BLUE_Pin        GPIO_PIN_15
#define LED_ALL_Pins        (LED_GREEN_Pin | LED_ORANGE_Pin | LED_RED_Pin | LED_BLUE_Pin)
#define LED_GPIO_Port       GPIOD

/* USART2: PA2 TX, PA3 RX, AF7 */
#define USART2_TX_Pin       GPIO_PIN_2
#define USART2_RX_Pin       GPIO_PIN_3
#define USART2_GPIO_Port    GPIOA
#define USART2_BAUDRATE     115200U

/*
 * FreeRTOS ...FromISR API'lerini cagiracak kesmeler icin NVIC onceligi.
 * configMAX_SYSCALL_INTERRUPT_PRIORITY (tipik olarak 5) degerinden sayisal
 * olarak buyuk veya esit olmali; aksi halde FromISR cagrilari guvensizdir.
 */
#define APP_IRQ_PRIORITY    6U
/* Buton EXTI'si UART kesmesini de kesebilsin diye bir kademe yuksek (FromISR icin izin verilen en yuksek) */
#define BUTTON_IRQ_PRIORITY 5U

/* UART'in tek sahibi UartTxTask olacak; handle sadece onun icin disa acik */
extern UART_HandleTypeDef huart2;

/* HAL zaman tabani (stm32f4xx_hal_timebase_tim.c) */
extern TIM_HandleTypeDef htim7;

void SystemClock_Config(void);
void MX_LED_GPIO_Init(void);
void MX_Button_EXTI_Init(void);
void MX_USART2_UART_Init(void);
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */
