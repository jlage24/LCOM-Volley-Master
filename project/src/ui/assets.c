#include "assets.h"

#include "../images/menu.xpm"
#include "../images/menu_pause.xpm"
#include "../images/btn_start.xpm"
#include "../images/btn_exit.xpm"
#include "../images/btn_resume.xpm"
#include "../images/btn_menu.xpm"
#include "../images/players/eagle.xpm"
#include "../images/players/panda.xpm"
#include "../images/ball.xpm"

#include <stdlib.h>

xpm_image_t bg_pixmap;
xpm_image_t menu_pixmap;
xpm_image_t menu_pause_pixmap;
xpm_image_t beach_volley_pixmap;
xpm_image_t desert_volley_pixmap;
xpm_image_t jungle_volley_pixmap;
xpm_image_t ice_volley_pixmap;

xpm_image_t btn_start_pixmap;
xpm_image_t btn_exit_pixmap;
xpm_image_t btn_resume_pixmap;
xpm_image_t btn_menu_pixmap;
xpm_image_t player1_pixmap;
xpm_image_t player2_pixmap;
xpm_image_t ball_pixmap;

static int load_xpm_asset(xpm_map_t xpm, xpm_image_t *img) {
    return xpm_load(xpm, XPM_8_8_8_8, img) == NULL;
}

static void free_xpm_asset(xpm_image_t *img) {
    if (img->bytes != NULL) {
        free(img->bytes);
        img->bytes = NULL;
    }
}

int load_all_assets() {
    if (load_xpm_asset((xpm_map_t) menu, &menu_pixmap)) return 1;
    if (load_xpm_asset((xpm_map_t) menu_pause, &menu_pause_pixmap)) return 1;

    if (load_xpm_asset((xpm_map_t) btn_start, &btn_start_pixmap)) return 1;
    if (load_xpm_asset((xpm_map_t) btn_exit, &btn_exit_pixmap)) return 1;

    if (load_xpm_asset((xpm_map_t) btn_resume, &btn_resume_pixmap)) return 1;
    if (load_xpm_asset((xpm_map_t) btn_menu, &btn_menu_pixmap)) return 1;
    if (load_xpm_asset((xpm_map_t) eagle_xpm, &player1_pixmap)) return 1;
    if (load_xpm_asset((xpm_map_t) panda_xpm, &player2_pixmap)) return 1;
    if (load_xpm_asset((xpm_map_t) ball_xpm, &ball_pixmap)) return 1;

    load_field_assets();

    return 0;
}

void free_all_assets() {
    free_xpm_asset(&menu_pixmap);
    free_xpm_asset(&menu_pause_pixmap);
    free_xpm_asset(&beach_volley_pixmap);
    free_xpm_asset(&desert_volley_pixmap);
    free_xpm_asset(&jungle_volley_pixmap);
    free_xpm_asset(&ice_volley_pixmap);
    free_xpm_asset(&btn_start_pixmap);
    free_xpm_asset(&btn_exit_pixmap);
    free_xpm_asset(&btn_resume_pixmap);
    free_xpm_asset(&btn_menu_pixmap);
    free_xpm_asset(&player1_pixmap);
    free_xpm_asset(&player2_pixmap);
    free_xpm_asset(&ball_pixmap);
}

