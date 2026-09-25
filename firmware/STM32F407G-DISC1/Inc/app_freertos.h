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

/* Buton ISR'inin buttonQ'ya FromISR ile gonderdigi olay */
typedef struct
{
    uint32_t id;        /* basis sira numarasi (BTN,<id>,...) */
    uint32_t t0_us;     /* ISR girisindeki timer_us() degeri */
} button_evt_t;

/* txQ elemani: sabit 64 bayt mesaj */
typedef struct
{
    char data[TX_MSG_SIZE];
} tx_msg_t;

_Static_assert(sizeof(tx_msg_t) == TX_MSG_SIZE, "tx_msg_t tam 64 bayt olmali");

extern osMessageQueueId_t buttonQ;
extern osMessageQueueId_t txQ;

/* Kuyruklari ve gorevleri olusturur; osKernelInitialize'dan sonra, osKernelStart'tan once cagrilir */
void MX_FREERTOS_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_FREERTOS_H */
