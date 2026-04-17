#include <lcom/lcf.h>

#include <stdint.h>
#include "utils.h"

static uint32_t cnt = 0;

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  if (lsb == NULL) return 1;
  *lsb = val & 0xFF;
  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  if (msb == NULL) return 1;
  *msb = (val >> 8) & 0xFF;
  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {
  if (value == NULL) return 1;
  uint32_t temp;
  if (sys_inb(port, &temp) != 0) return 1;
  *value = (uint8_t) temp;
  cnt++;
  return 0;
}

uint32_t get_sys_inb_cnt() {
    return cnt;
}
