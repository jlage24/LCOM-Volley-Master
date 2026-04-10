#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

static int hook_id_timer = 0;
static uint32_t timer_ticks = 0;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  if (timer > 2) return 1;
  if (freq < 19 || freq > 1193181) return 1; 
  uint8_t st;
  if (timer_get_conf(timer, &st) != 0) return 1;
  st = st & 0x0F;
  uint8_t ctrl_word = (timer << 6) | BIT(5) | BIT(4) | st;
  if (sys_outb(TIMER_CTRL, ctrl_word) != 0) return 1;
  uint16_t div = TIMER_FREQ / freq; 
  uint8_t lsb, msb;
  util_get_LSB(div, &lsb);
  util_get_MSB(div, &msb);
  if (sys_outb(TIMER_0 + timer, lsb) != 0) return 1;
  if (sys_outb(TIMER_0 + timer, msb) != 0) return 1;
  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
  if (bit_no == NULL) return 1;
  *bit_no = hook_id_timer; 
  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id_timer) != 0) {
      return 1;
  }
  return 0;
}

int (timer_unsubscribe_int)() {
  if (sys_irqrmpolicy(&hook_id_timer) != 0) {
      return 1;
  }
  return 0;
}

void (timer_int_handler)() {
  timer_ticks++;
}

uint32_t get_timer_ticks() {
  return timer_ticks;
}

void reset_timer_ticks() {
  timer_ticks = 0;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  if (timer > 2) return 1;
  uint8_t read_back_cmd = (BIT(7) | BIT(6) | BIT(5) | BIT(timer + 1));
  // Send to Control Register (0x43)
  if (sys_outb(0x43, read_back_cmd) != 0) return 1;
  // Read the status from the respective timer port (0x40, 0x41, 0x42)
  if (util_sys_inb(0x40 + timer, st) != 0) return 1;
  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {
  union timer_status_field_val val;
  switch (field) {
      case tsf_all:
          val.byte = st;
          break;
      case tsf_initial:
          val.in_mode = (st >> 4) & 0x03; 
          break;
      case tsf_mode:
          val.count_mode = (st >> 1) & 0x07;
          if (val.count_mode == 6) val.count_mode = 2;
          if (val.count_mode == 7) val.count_mode = 3;
          break;
      case tsf_base:
          val.bcd = st & 0x01;
          break;
      default:
          return 1;
  }
  return timer_print_config(timer, field, val);
}
