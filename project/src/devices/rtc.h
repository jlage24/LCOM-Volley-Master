#pragma once

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
