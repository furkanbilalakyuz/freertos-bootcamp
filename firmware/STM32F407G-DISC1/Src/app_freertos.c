/**
 ******************************************************************************
 * @file           : app_freertos.c
 * @brief          : Uygulama gorevleri ve kuyruklari (CMSIS-RTOS2)
 *
 * Mimari (CLAUDE.md):
 *   TelemetryTask (en yuksek) -> txQ
 *   ButtonTask    (orta)      buttonQ -> txQ
 *   UartTxTask    (en dusuk)  txQ -> UART (UART'in tek sahibi)
 ******************************************************************************
 */

#include <stdbool.h>
#include "app_freertos.h"
#include "main.h"
#include "timer_us.h"
#include "FreeRTOS.h"
#include "task.h"

osMessageQueueId_t buttonQ;
osMessageQueueId_t txQ;

volatile uint32_t button_drop_count;

/* Yalnizca button_isr kullanir (tek kesme, ic ice girmez) */
static uint32_t button_last_accept_us;
static uint32_t button_next_id = 1U;
static bool button_has_accepted;

static osThreadId_t telemetryTaskHandle;
static osThreadId_t buttonTaskHandle;
static osThreadId_t uartTxTaskHandle;

static void TelemetryTask(void *argument);
static void ButtonTask(void *argument);
static void UartTxTask(void *argument);

/* CMSIS-RTOS2'de osPriority dogrudan FreeRTOS onceligidir; yalnizca siralama onemli */
static const osThreadAttr_t telemetryTask_attr = {
    .name = "TelemetryTask",
    .stack_size = 2048,
    .priority = osPriorityAboveNormal,
};

static const osThreadAttr_t buttonTask_attr = {
    .name = "ButtonTask",
    .stack_size = 2048,
    .priority = osPriorityNormal,
};

static const osThreadAttr_t uartTxTask_attr = {
    .name = "UartTxTask",
    .stack_size = 1024,
    .priority = osPriorityBelowNormal,
};

static const osMessageQueueAttr_t buttonQ_attr = {
    .name = "buttonQ",
};

static const osMessageQueueAttr_t txQ_attr = {
    .name = "txQ",
};

void MX_FREERTOS_Init(void)
{
    buttonQ = osMessageQueueNew(BUTTON_QUEUE_LEN, sizeof(button_evt_t), &buttonQ_attr);
    txQ = osMessageQueueNew(TX_QUEUE_LEN, sizeof(tx_msg_t), &txQ_attr);
    if ((buttonQ == NULL) || (txQ == NULL))
    {
        Error_Handler();
    }

    telemetryTaskHandle = osThreadNew(TelemetryTask, NULL, &telemetryTask_attr);
    buttonTaskHandle = osThreadNew(ButtonTask, NULL, &buttonTask_attr);
    uartTxTaskHandle = osThreadNew(UartTxTask, NULL, &uartTxTask_attr);
    if ((telemetryTaskHandle == NULL) || (buttonTaskHandle == NULL) || (uartTxTaskHandle == NULL))
    {
        Error_Handler();
    }
}

/*
 * Tekrar-kenar filtresi: pencere son KABUL edilen basistan olculur. Ilk basista
 * karsilastirilacak onceki kenar olmadigi icin dogrudan kabul edilir. Pencere
 * icindeki sicrama kenarlari pencereyi uzatmaz; sadece yok sayilir.
 */
void button_isr(uint32_t t0_us)
{
    button_evt_t evt;

    if (button_has_accepted &&
        (timer_us_elapsed(button_last_accept_us, t0_us) < BUTTON_DEBOUNCE_US))
    {
        return;
    }

    button_has_accepted = true;
    button_last_accept_us = t0_us;

    evt.id = button_next_id++;
    evt.t0_us = t0_us;

    /* ISR'de timeout 0 olmali; sarmalayici FromISR + portYIELD_FROM_ISR kullanir */
    if (osMessageQueuePut(buttonQ, &evt, 0U, 0U) != osOK)
    {
        button_drop_count++;
    }
}

/* Periyodik telemetri mesaji uretip txQ'ya yazacak */
static void TelemetryTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        /* Govde henuz yok; osDelay, dusuk oncelikli gorevlerin de CPU alabilmesi icin */
        osDelay(1000);
    }
}

/* buttonQ'dan olay alip (t1) yanit mesajini txQ'ya yazacak (t2) */
static void ButtonTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        osDelay(1000);
    }
}

/* txQ'yu tuketip UART'a gonderecek (t3); UART'a erisen tek gorev */
static void UartTxTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        osDelay(1000);
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    Error_Handler();
}

void vApplicationMallocFailedHook(void)
{
    Error_Handler();
}

void vAssertCalled(const char *file, int line)
{
    (void)file;
    (void)line;
    Error_Handler();
}
