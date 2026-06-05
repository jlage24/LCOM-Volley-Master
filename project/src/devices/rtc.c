#include "rtc.h"

#define TODO return -1

#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71
#define RTC_REG_SECONDS 0x00
#define RTC_REG_MINUTES 0x02
#define RTC_REG_HOURS 0x04
#define RTC_REG_A 0x0A
#define RTC_REG_B 0x0B
#define RTC_REG_DAY 0x07
#define RTC_REG_MONTH 0x08
#define RTC_REG_YEAR 0x09
#define RTC_UIP_MSK (1 << 7)
#define RTC_DM_MSK (1 << 2)
#define RTC_24H_MSK (1 << 1)
#define RTC_PM_MSK (1 << 7)

static int bcd_to_bin(uint8_t bcd) { 
  return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

int rtc_read_date(rtc_date *date) { 
  if (date == NULL) return -1;

  uint32_t reg_a;
  do {
    if (sys_outb(RTC_ADDR_REG, RTC_REG_A) != OK) return -1;
    if (sys_inb(RTC_DATA_REG, &reg_a)     != OK) return -1;
    tickdelay(micros_to_ticks(1000));
  } while (reg_a & RTC_UIP_MSK);
  uint32_t seconds, minutes, hours, day, month, year;
  if (sys_outb(RTC_ADDR_REG, RTC_REG_SECONDS) != OK) return -1;
  if (sys_inb(RTC_DATA_REG,  &seconds)         != OK) return -1;
  if (sys_outb(RTC_ADDR_REG, RTC_REG_MINUTES) != OK) return -1;
  if (sys_inb(RTC_DATA_REG,  &minutes)         != OK) return -1;
  if (sys_outb(RTC_ADDR_REG, RTC_REG_HOURS)   != OK) return -1;
  if (sys_inb(RTC_DATA_REG,  &hours)           != OK) return -1;
  if (sys_outb(RTC_ADDR_REG, RTC_REG_DAY)   != OK) return -1;
  if (sys_inb(RTC_DATA_REG,  &day)           != OK) return -1;
  if (sys_outb(RTC_ADDR_REG, RTC_REG_MONTH) != OK) return -1;
  if (sys_inb(RTC_DATA_REG,  &month)         != OK) return -1;
  if (sys_outb(RTC_ADDR_REG, RTC_REG_YEAR)  != OK) return -1;
  if (sys_inb(RTC_DATA_REG,  &year)          != OK) return -1;
  uint32_t reg_b;
  if (sys_outb(RTC_ADDR_REG, RTC_REG_B) != OK) return -1;
  if (sys_inb(RTC_DATA_REG,  &reg_b)    != OK) return -1;

  uint8_t raw_hours = (uint8_t) hours;

  if (!(reg_b & RTC_DM_MSK)) {  
    seconds = bcd_to_bin((uint8_t) seconds);
    minutes = bcd_to_bin((uint8_t) minutes);
    hours = bcd_to_bin((uint8_t) hours & ~RTC_PM_MSK);
    day   = bcd_to_bin((uint8_t) day);
    month = bcd_to_bin((uint8_t) month);
    year  = bcd_to_bin((uint8_t) year);
  } else {
    hours &= ~RTC_PM_MSK;
  }

  if (!(reg_b & RTC_24H_MSK)) {
    if (raw_hours & RTC_PM_MSK) {
      hours = ((hours % 12) + 12);
    } else if (hours == 12) {
      hours = 0;
    }
  }

  date->hours = (uint8_t) hours;
  date->minutes = (uint8_t) minutes;
  date->seconds = (uint8_t) seconds;
  date->day   = (uint8_t) day;
  date->month = (uint8_t) month;
  date->year  = (uint8_t) year;
  return 0; 
}
