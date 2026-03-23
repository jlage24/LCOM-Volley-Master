#include "rtc.h"

#define TODO return -1

#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71
#define RTC_REG_A 0x0A
#define RTC_REG_B 0x0B
#define RTC_REG_DAY 0x07
#define RTC_REG_MONTH 0x08
#define RTC_REG_YEAR 0x09
#define RTC_UIP_MSK (1 << 7)
#define RTC_DM_MSK (1 << 2)

static int bcd_to_bin(uint8_t bcd) { 
  return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

int rtc_read_date(rtc_date *date) { 
  uint32_t reg_a;
  do {
    if (sys_outb(RTC_ADDR_REG, RTC_REG_A) != OK) return -1;
    if (sys_inb(RTC_DATA_REG, &reg_a)     != OK) return -1;
    tickdelay(micros_to_ticks(1000));
  } while (reg_a & RTC_UIP_MSK);
  uint32_t day, month, year;
  if (sys_outb(RTC_ADDR_REG, RTC_REG_DAY)   != OK) return -1;
  if (sys_inb(RTC_DATA_REG,  &day)           != OK) return -1;
  if (sys_outb(RTC_ADDR_REG, RTC_REG_MONTH) != OK) return -1;
  if (sys_inb(RTC_DATA_REG,  &month)         != OK) return -1;
  if (sys_outb(RTC_ADDR_REG, RTC_REG_YEAR)  != OK) return -1;
  if (sys_inb(RTC_DATA_REG,  &year)          != OK) return -1;
  uint32_t reg_b;
  if (sys_outb(RTC_ADDR_REG, RTC_REG_B) != OK) return -1;
  if (sys_inb(RTC_DATA_REG,  &reg_b)    != OK) return -1;
  if (!(reg_b & RTC_DM_MSK)) {  
    day   = bcd_to_bin((uint8_t) day);
    month = bcd_to_bin((uint8_t) month);
    year  = bcd_to_bin((uint8_t) year);
  }
  date->day   = (uint8_t) day;
  date->month = (uint8_t) month;
  date->year  = (uint8_t) year;
  return 0; 
}
