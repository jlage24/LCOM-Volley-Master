#ifndef _ASSETS_H_
#define _ASSETS_H_

#include <lcom/lcf.h>

extern xpm_image_t bg_pixmap;

extern xpm_image_t menu_pixmap;
extern xpm_image_t menu_pause_pixmap;
extern xpm_image_t beach_volley_pixmap;
extern xpm_image_t desert_volley_pixmap;
extern xpm_image_t jungle_volley_pixmap;
extern xpm_image_t ice_volley_pixmap;

extern xpm_image_t btn_start_pixmap;
extern xpm_image_t btn_exit_pixmap;
extern xpm_image_t btn_resume_pixmap;
extern xpm_image_t btn_menu_pixmap;

int load_all_assets();
void load_field_assets();

#endif

