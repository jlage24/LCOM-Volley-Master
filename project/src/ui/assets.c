#include "assets.h"

#include "../images/menu.xpm"
#include "../images/menu_pause.xpm"
#include "../images/selection_screen_jungle_1024x768.xpm"
#include "../images/locked_character.xpm"
#include "../images/btn_start.xpm"
#include "../images/btn_exit.xpm"
#include "../images/btn_resume.xpm"
#include "../images/btn_menu.xpm"
#include "../images/players/eagle.xpm"
#include "../images/players/panda.xpm"
#include "../images/players/jaguar.xpm"
#include "../images/players/lion.xpm"
#include "../images/players/bear.xpm"
#include "../images/players/wolf.xpm"

#include <stdlib.h>

xpm_image_t bg_pixmap;
xpm_image_t menu_pixmap;
xpm_image_t menu_pause_pixmap;
xpm_image_t character_select_pixmap;
xpm_image_t locked_character_pixmap;
xpm_image_t beach_volley_pixmap;
xpm_image_t desert_volley_pixmap;
xpm_image_t jungle_volley_pixmap;
xpm_image_t ice_volley_pixmap;

xpm_image_t btn_start_pixmap;
xpm_image_t btn_exit_pixmap;
xpm_image_t btn_resume_pixmap;
xpm_image_t btn_menu_pixmap;
xpm_image_t player_pixmaps[PLAYER_CHARACTER_COUNT];
xpm_image_t *player1_pixmap = &player_pixmaps[0];
xpm_image_t *player2_pixmap = &player_pixmaps[1];
const char *player_names[PLAYER_CHARACTER_COUNT] = {
    "EAGLE",
    "PANDA",
    "JAGUAR",
    "LION",
    "BEAR",
    "WOLF"
};

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
    if (load_xpm_asset((xpm_map_t) selection_screen_jungle_1024x768_xpm, &character_select_pixmap)) return 1;
    if (load_xpm_asset((xpm_map_t) locked_character_xpm, &locked_character_pixmap)) return 1;

    if (load_xpm_asset((xpm_map_t) btn_start, &btn_start_pixmap)) return 1;
    if (load_xpm_asset((xpm_map_t) btn_exit, &btn_exit_pixmap)) return 1;

    if (load_xpm_asset((xpm_map_t) btn_resume, &btn_resume_pixmap)) return 1;
    if (load_xpm_asset((xpm_map_t) btn_menu, &btn_menu_pixmap)) return 1;
    if (load_xpm_asset((xpm_map_t) eagle_xpm, &player_pixmaps[0])) return 1;
    if (load_xpm_asset((xpm_map_t) panda_xpm, &player_pixmaps[1])) return 1;
    if (load_xpm_asset((xpm_map_t) jaguar_xpm, &player_pixmaps[2])) return 1;
    if (load_xpm_asset((xpm_map_t) lion_xpm, &player_pixmaps[3])) return 1;
    if (load_xpm_asset((xpm_map_t) bear_xpm, &player_pixmaps[4])) return 1;
    if (load_xpm_asset((xpm_map_t) wolf_xpm, &player_pixmaps[5])) return 1;

    player1_pixmap = &player_pixmaps[0];
    player2_pixmap = &player_pixmaps[1];

    load_field_assets();

    return 0;
}

void free_all_assets() {
    free_xpm_asset(&menu_pixmap);
    free_xpm_asset(&menu_pause_pixmap);
    free_xpm_asset(&character_select_pixmap);
    free_xpm_asset(&locked_character_pixmap);
    free_xpm_asset(&beach_volley_pixmap);
    free_xpm_asset(&desert_volley_pixmap);
    free_xpm_asset(&jungle_volley_pixmap);
    free_xpm_asset(&ice_volley_pixmap);
    free_xpm_asset(&btn_start_pixmap);
    free_xpm_asset(&btn_exit_pixmap);
    free_xpm_asset(&btn_resume_pixmap);
    free_xpm_asset(&btn_menu_pixmap);
    for (int i = 0; i < PLAYER_CHARACTER_COUNT; i++) {
        free_xpm_asset(&player_pixmaps[i]);
    }

    player1_pixmap = &player_pixmaps[0];
    player2_pixmap = &player_pixmaps[1];
}
