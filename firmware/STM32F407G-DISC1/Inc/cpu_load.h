/**
 ******************************************************************************
 * @file           : cpu_load.h
 * @brief          : S4/S5 senaryolari icin kalibre edilmis CPU yuku
 ******************************************************************************
 */

#ifndef CPU_LOAD_H
#define CPU_LOAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * Iterasyon/us oranini timer_us ile olcer. timer_us_init'ten sonra, zamanlayici
 * baslamadan once bir kez cagrilir; olcum sirasinda kesmeler kapatilir (birkac ms).
 */
void calibrated_work_calibrate(void);

/*
 * Kesintisiz calistiginda yaklasik `us` mikrosaniye suren gercek hesaplama yapar.
 * Zamana degil is miktarina baglidir: gorev araya giren kesme/gorevlerle
 * kesilirse duvar saati suresi uzar, yapilan is ayni kalir.
 * Sonuc hem dondurulur hem calibrated_work_sink'e yazilir; derleyici silemez.
 */
uint32_t calibrated_work(uint32_t us);

/* Kalibrasyon sonucu: 1 us'deki iterasyon sayisi, Q16 sabit nokta */
extern uint32_t calibrated_work_iters_per_us_q16;
extern volatile uint32_t calibrated_work_sink;

#ifdef __cplusplus
}
#endif

#endif /* CPU_LOAD_H */
