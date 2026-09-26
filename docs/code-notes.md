# Kod Notlari

## Ilk uctan uca dogrulama (ST-Link debugger, 2026-09-26)

Bu, sistemin debugger uzerinden yapilan ilk basarili uctan uca dogrulamasidir.
Buton ISR'indan UART TC callback'ine kadar tum zincir (ISR -> buttonQ ->
ButtonTask -> txQ -> UartTxTask -> UART TC) kart uzerinde calisti ve zaman
damgalari olay kaydina yazildi.

### Gozlemler

- `tel_log_count` zaman icinde artiyor: TelemetryTask periyodik olarak
  calisiyor ve mesajlari UartTxTask uzerinden gonderilip kaydediliyor.
- `btn_log_count` her buton basisinda artiyor: ISR, ButtonTask ve UartTxTask
  zinciri buton olaylarini isleyip kayda yaziyor.

### Ornek kayit

GDB'de okunan son BTN kaydi:

```
$3 = {meta = {kind = 2, scenario = 1, reserved = 0, id = 14,
      t0_us = 194753350, t1_us = 194753361, t2_us = 194753367},
      t3_us = 194753393, t4_us = 194758950}
```

### Asama sureleri

| Aralik  | Asama                                                        | Sure (us) |
|---------|--------------------------------------------------------------|----------:|
| t1 - t0 | ISR girisi -> ButtonTask `xQueueReceive` donusu (buttonQ)    |        11 |
| t2 - t1 | ButtonTask mesaj hazirlama (BTN satiri formatlama)           |         6 |
| t3 - t2 | `xQueueSend` (txQ) + UartTxTask'a gecis -> UART baslatma oncesi |      26 |
| t4 - t3 | UART aktarimi (64 bayt) -> TC callback                       |      5557 |
| **t4 - t0** | **Toplam yanit suresi R**                                | **5600**  |

R = 5600 us (~5.6 ms), 20 ms deadline'in rahatca altinda (~%28'i).

### Yorum

- Yazilim tarafi (t0 -> t3) toplam yalnizca 43 us surdu; yanit suresinin
  %99'dan fazlasi UART aktariminda geciyor.
- t4 - t3 = 5557 us, teorik aktarim suresiyle neredeyse birebir ortusuyor:
  64 bayt x 10 bit (8N1) / 115200 baud = 5555.6 us. Aradaki ~1.4 us farki
  HAL IT baslatma ve TC kesmesine giris maliyetidir.
- t3 - t2 (26 us) araligi, ButtonTask'in (oncelik 2) txQ'ya yazip bloklanmasi
  ve dusuk oncelikli UartTxTask'in (oncelik 1) calismaya baslamasini icerir;
  bu nedenle yazilim asamalari arasinda en uzunudur.
- Bu olcum tek bir ornektir (senaryo 1, id 14). Dagilim, en kotu durum ve
  yuk altindaki davranis (S4/S5) icin cok sayida ornek toplanmasi gerekir.
  Ayrica bu kayitta txQ bos oldugu icin kuyrukta bekleme gorulmuyor; TEL
  mesajlari kuyrukta onde oldugunda t3 - t2 bir veya birkac mesaj suresi
  (~5.6 ms/mesaj) kadar uzayabilir.

### Henuz yapilmayanlar

- UART/PC testi henuz yapilmadi. Dogrulama yalnizca debugger uzerinden olay
  kaydi okunarak yapildi; UART hattindan cikan baytlar PC tarafinda henuz
  alinip dogrulanmadi (mesaj formati, 64 bayt uzunlugu, LF sonlandirma,
  kayip/bozulma kontrolu).
