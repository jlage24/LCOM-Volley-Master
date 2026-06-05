#pragma once

/*
 * RTC date/time interface.
 *
 * rtc.c reads the hardware RTC, normalizes BCD/12-hour formats, and returns a
 * compact date structure used by the menu HUD.
 */
#include <stdint.h>
#include <lcom/lcf.h>

typedef struct {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint8_t day;
  uint8_t month;
  uint8_t year;
} rtc_date;

/* Reads the current RTC date into `date`. Returns 0 on success. */
int rtc_read_date(rtc_date *date);
