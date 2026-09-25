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
#include <string.h>
#include "app_freertos.h"
#include "main.h"
#include "timer_us.h"
#include "FreeRTOS.h"
#include "task.h"

/* UART TC callback'inin UartTxTask'i uyandirdigi thread flag */
#define UART_TX_DONE_FLAG       0x0001U
/* 64 bayt @115200 8N1 ~5.6 ms; bunun cok ustu donanim/surucu hatasidir */
#define UART_TX_TIMEOUT_MS      20U

osMessageQueueId_t buttonQ;
osMessageQueueId_t txQ;

volatile uint8_t app_scenario = 1U;
volatile uint32_t telemetry_period_ms = TELEMETRY_PERIOD_DEFAULT_MS;

evt_log_t evt_log[EVT_LOG_LEN];
volatile uint32_t evt_log_count;

volatile uint32_t button_drop_count;
volatile uint32_t tel_drop_count;
volatile uint32_t uart_err_count;

/* Yalnizca button_isr kullanir (tek kesme, ic ice girmez) */
static uint32_t button_last_accept_us;
static uint32_t button_next_id = 1U;
static bool button_has_accepted;

/* UART'ta o an gonderilen eleman; IT transferi bitene kadar bellekte kalmali */
static tx_item_t tx_inflight;
/* TC callback'inde yazilir, UartTxTask'ta okunur */
static volatile uint32_t tx_t4_us;

static osThreadId_t telemetryTaskHandle;
static osThreadId_t buttonTaskHandle;
static osThreadId_t uartTxTaskHandle;

static void TelemetryTask(void *argument);
static void ButtonTask(void *argument);
static void UartTxTask(void *argument);

/* CMSIS-RTOS2'de osPriority dogrudan FreeRTOS onceligidir; yalnizca siralama onemli */
static const osThreadAttr_t telemetryTask_attr = {
    .name = "TelemetryTask",
    .stack_size = 1024,
    .priority = osPriorityAboveNormal,
};

static const osThreadAttr_t buttonTask_attr = {
    .name = "ButtonTask",
    .stack_size = 1024,
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
    txQ = osMessageQueueNew(TX_QUEUE_LEN, sizeof(tx_item_t), &txQ_attr);
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

/*
 * Mesaj bicimlendirme. snprintf kullanilmiyor: configUSE_NEWLIB_REENTRANT 0 iken
 * farkli oncelikli gorevlerden cagrilmasi guvenli degil, suresi de sabit degil.
 */
static size_t fmt_str(char *dst, size_t pos, const char *s)
{
    while ((*s != '\0') && (pos < (TX_MSG_SIZE - 1U)))
    {
        dst[pos++] = *s++;
    }
    return pos;
}

static size_t fmt_u32(char *dst, size_t pos, uint32_t v)
{
    char tmp[10];
    size_t n = 0U;

    do
    {
        tmp[n++] = (char)('0' + (v % 10U));
        v /= 10U;
    } while (v != 0U);

    while ((n > 0U) && (pos < (TX_MSG_SIZE - 1U)))
    {
        dst[pos++] = tmp[--n];
    }
    return pos;
}

/* 63 bayta kadar bosluk doldurur, 64. bayt LF; sonlandirici NUL yok */
static void fmt_finish(tx_msg_t *msg, size_t pos)
{
    memset(&msg->data[pos], ' ', (TX_MSG_SIZE - 1U) - pos);
    msg->data[TX_MSG_SIZE - 1U] = '\n';
}

/* TEL,<seq>,S<senaryo>,<ms> */
static void format_tel(tx_msg_t *msg, uint32_t seq, uint8_t scenario, uint32_t ms)
{
    size_t pos = 0U;

    pos = fmt_str(msg->data, pos, "TEL,");
    pos = fmt_u32(msg->data, pos, seq);
    pos = fmt_str(msg->data, pos, ",S");
    pos = fmt_u32(msg->data, pos, scenario);
    pos = fmt_str(msg->data, pos, ",");
    pos = fmt_u32(msg->data, pos, ms);
    fmt_finish(msg, pos);
}

/* BTN,<id>,S<senaryo>,PRESSED */
static void format_btn(tx_msg_t *msg, uint32_t id, uint8_t scenario)
{
    size_t pos = 0U;

    pos = fmt_str(msg->data, pos, "BTN,");
    pos = fmt_u32(msg->data, pos, id);
    pos = fmt_str(msg->data, pos, ",S");
    pos = fmt_u32(msg->data, pos, scenario);
    pos = fmt_str(msg->data, pos, ",PRESSED");
    fmt_finish(msg, pos);
}

/*
 * Periyodik TEL mesaji. osDelayUntil ile periyot kaymaz; telemetry_period_ms
 * degisirse bir sonraki periyottan itibaren uygulanir. txQ doluysa beklemez
 * (periyodu bozmamak icin), mesaji atlar ve tel_drop_count'u artirir.
 */
static void TelemetryTask(void *argument)
{
    tx_item_t item;
    uint32_t seq = 0U;
    uint32_t next_wake = osKernelGetTickCount();

    (void)argument;

    for (;;)
    {
        next_wake += telemetry_period_ms;
        osDelayUntil(next_wake);

        seq++;
        item.meta.kind = MSG_KIND_TEL;
        item.meta.scenario = app_scenario;
        item.meta.reserved = 0U;
        item.meta.id = seq;
        item.meta.t0_us = 0U;
        item.meta.t1_us = 0U;
        /* configTICK_RATE_HZ 1000 oldugu icin tick = ms */
        format_tel(&item.msg, seq, item.meta.scenario, osKernelGetTickCount());

        item.meta.t2_us = timer_us();
        if (osMessageQueuePut(txQ, &item, 0U, 0U) != osOK)
        {
            tel_drop_count++;
        }
    }
}

/*
 * buttonQ'dan olay al (t1), BTN mesajini txQ'ya yaz (t2). Buton olayi kaybolmasin
 * diye txQ doluysa bekler; bu sirada daha dusuk oncelikli UartTxTask kuyrugu bosaltir.
 */
static void ButtonTask(void *argument)
{
    button_evt_t evt;
    tx_item_t item;

    (void)argument;

    for (;;)
    {
        if (osMessageQueueGet(buttonQ, &evt, NULL, osWaitForever) != osOK)
        {
            continue;
        }
        item.meta.t1_us = timer_us();

        item.meta.kind = MSG_KIND_BTN;
        item.meta.scenario = app_scenario;
        item.meta.reserved = 0U;
        item.meta.id = evt.id;
        item.meta.t0_us = evt.t0_us;
        format_btn(&item.msg, evt.id, item.meta.scenario);

        item.meta.t2_us = timer_us();
        (void)osMessageQueuePut(txQ, &item, 0U, osWaitForever);
    }
}

static void evt_log_append(const tx_meta_t *meta, uint32_t t3_us, uint32_t t4_us)
{
    evt_log_t *e = &evt_log[evt_log_count & (EVT_LOG_LEN - 1U)];

    e->meta = *meta;
    e->t3_us = t3_us;
    e->t4_us = t4_us;
    /* Girdi tamamen yazildiktan sonra sayac ilerlesin (hata ayiklayici/okuyucu icin) */
    __DMB();
    evt_log_count++;
}

/*
 * txQ'yu tuketir, her mesaji USART2'ye IT ile gonderir (t3), TC callback'ini
 * (t4) bekler ve olayi RAM kaydina yazar. UART'a erisen tek gorev budur.
 */
static void UartTxTask(void *argument)
{
    uint32_t t3_us;
    uint32_t t4_us;
    uint32_t flags;

    (void)argument;

    for (;;)
    {
        if (osMessageQueueGet(txQ, &tx_inflight, NULL, osWaitForever) != osOK)
        {
            continue;
        }

        (void)osThreadFlagsClear(UART_TX_DONE_FLAG);
        tx_t4_us = 0U;

        t3_us = timer_us();
        if (HAL_UART_Transmit_IT(&huart2, (const uint8_t *)tx_inflight.msg.data, TX_MSG_SIZE) != HAL_OK)
        {
            uart_err_count++;
            evt_log_append(&tx_inflight.meta, t3_us, 0U);
            continue;
        }

        flags = osThreadFlagsWait(UART_TX_DONE_FLAG, osFlagsWaitAny, UART_TX_TIMEOUT_MS);
        if ((flags & osFlagsError) != 0U)
        {
            /* TC gelmedi: transferi iptal et ki sonraki mesaj baslatilabilsin */
            (void)HAL_UART_AbortTransmit(&huart2);
            uart_err_count++;
            t4_us = 0U;
        }
        else
        {
            t4_us = tx_t4_us;
        }

        evt_log_append(&tx_inflight.meta, t3_us, t4_us);
    }
}

/* HAL IT modunda bu callback TC (transfer complete) kesmesinde cagrilir */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uint32_t t4_us = timer_us();

    if (huart->Instance == USART2)
    {
        tx_t4_us = t4_us;
        (void)osThreadFlagsSet(uartTxTaskHandle, UART_TX_DONE_FLAG);
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
