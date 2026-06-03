#ifndef _ASSETS_H_
#define _ASSETS_H_

#include <lcom/lcf.h>

#define PLAYER_CHARACTER_COUNT 6

extern xpm_image_t bg_pixmap;

extern xpm_image_t menu_pixmap;
extern xpm_image_t menu_pause_pixmap;
extern xpm_image_t character_select_pixmap;
extern xpm_image_t locked_character_pixmap;
extern xpm_image_t beach_volley_pixmap;
extern xpm_image_t desert_volley_pixmap;
extern xpm_image_t jungle_volley_pixmap;
extern xpm_image_t ice_volley_pixmap;

extern xpm_image_t btn_start_pixmap;
extern xpm_image_t btn_exit_pixmap;
extern xpm_image_t btn_resume_pixmap;
extern xpm_image_t btn_menu_pixmap;
extern xpm_image_t player_pixmaps[PLAYER_CHARACTER_COUNT];
extern xpm_image_t *player1_pixmap;
extern xpm_image_t *player2_pixmap;
extern const char *player_names[PLAYER_CHARACTER_COUNT];

int load_all_assets();
void free_all_assets();
void load_field_assets();

#endif
