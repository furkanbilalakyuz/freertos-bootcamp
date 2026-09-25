/**
 ******************************************************************************
 * @file           : app_freertos.h
 * @brief          : Uygulama gorevleri ve kuyruklari (CMSIS-RTOS2)
 ******************************************************************************
 */

#ifndef APP_FREERTOS_H
#define APP_FREERTOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "cmsis_os2.h"

#define BUTTON_QUEUE_LEN    8U
#define TX_QUEUE_LEN        16U
#define TX_MSG_SIZE         64U     /* 63 bayt icerik (bosluk dolgulu) + LF */
#define BUTTON_DEBOUNCE_US  30000U  /* kabul edilen basistan sonraki tekrar-kenar penceresi */

#define TELEMETRY_PERIOD_DEFAULT_MS  100U
#define EVT_LOG_LEN         64U     /* 2'nin kuvveti olmali (indeks maskeleme) */

/* Buton ISR'inin buttonQ'ya FromISR ile gonderdigi olay */
typedef struct
{
    uint32_t id;        /* basis sira numarasi (BTN,<id>,...) */
    uint32_t t0_us;     /* ISR girisindeki timer_us() degeri */
} button_evt_t;

/* UART'a giden sabit 64 bayt mesaj */
typedef struct
{
    char data[TX_MSG_SIZE];
} tx_msg_t;

_Static_assert(sizeof(tx_msg_t) == TX_MSG_SIZE, "tx_msg_t tam 64 bayt olmali");

typedef enum
{
    MSG_KIND_TEL = 1,
    MSG_KIND_BTN = 2,
} msg_kind_t;

/* Mesajla birlikte tasinan olcum bilgisi; UART'a gonderilmez */
typedef struct
{
    uint8_t  kind;      /* msg_kind_t */
    uint8_t  scenario;
    uint16_t reserved;
    uint32_t id;        /* TEL: seq, BTN: id */
    uint32_t t0_us;     /* BTN: ISR girisi, TEL: 0 */
    uint32_t t1_us;     /* BTN: buttonQ'dan alindiktan sonra, TEL: 0 */
    uint32_t t2_us;     /* txQ'ya gonderimden hemen once */
} tx_meta_t;

/* txQ elemani: 64 bayt mesaj + olcum bilgisi */
typedef struct
{
    tx_msg_t  msg;
    tx_meta_t meta;
} tx_item_t;

/* RAM olay kaydi girdisi; R = t4 - t0 (BTN) */
typedef struct
{
    tx_meta_t meta;
    uint32_t  t3_us;    /* UART baslatmadan hemen once */
    uint32_t  t4_us;    /* TC callback ani; 0 = gonderim tamamlanmadi (hata/zaman asimi) */
} evt_log_t;

extern osMessageQueueId_t buttonQ;
extern osMessageQueueId_t txQ;

/* Aktif senaryo numarasi (mesajlarda S<n> olarak gorunur) */
extern volatile uint8_t app_scenario;
/* TelemetryTask periyodu; bir sonraki periyottan itibaren gecerli olur */
extern volatile uint32_t telemetry_period_ms;

/* Olay kaydi: son EVT_LOG_LEN girdi, evt_log_count toplam yazilan girdi sayisi */
extern evt_log_t evt_log[EVT_LOG_LEN];
extern volatile uint32_t evt_log_count;

/* Hata/kayip sayaclari */
extern volatile uint32_t button_drop_count;  /* buttonQ dolu: kaybolan basis */
extern volatile uint32_t tel_drop_count;     /* txQ dolu: atlanan TEL mesaji */
extern volatile uint32_t uart_err_count;     /* UART baslatma hatasi veya TC zaman asimi */

/* EXTI0 ISR'inden cagrilir: 30 ms filtre + buttonQ'ya gonderim. t0_us: ISR girisindeki zaman */
void button_isr(uint32_t t0_us);

/* Kuyruklari ve gorevleri olusturur; osKernelInitialize'dan sonra, osKernelStart'tan once cagrilir */
void MX_FREERTOS_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_FREERTOS_H */
