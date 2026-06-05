#pragma once

/*
 * LCF entry points.
 *
 * proj_main_loop is implemented in proj.c and is the function invoked by the
 * LCOM framework after startup.
 */
int(proj_main_loop)(int argc, char *argv[]);

int(proj_demo)(uint16_t mode, bool minix3, bool grayscale, uint8_t delay);
