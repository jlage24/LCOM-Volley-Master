#include "assets.h"

#include "../images/menu.xpm"
#include "../images/menu_pause.xpm"
#include "../images/btn_start.xpm"
#include "../images/btn_exit.xpm"
#include "../images/btn_resume.xpm"
#include "../images/btn_menu.xpm"

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

int load_all_assets() {
    xpm_load((xpm_map_t) menu, XPM_8_8_8_8, &menu_pixmap);
    xpm_load((xpm_map_t) menu_pause, XPM_8_8_8_8, &menu_pause_pixmap);

    xpm_load((xpm_map_t) btn_start, XPM_8_8_8_8, &btn_start_pixmap);
    xpm_load((xpm_map_t) btn_exit, XPM_8_8_8_8, &btn_exit_pixmap);

    xpm_load((xpm_map_t) btn_resume, XPM_8_8_8_8, &btn_resume_pixmap);
    xpm_load((xpm_map_t) btn_menu, XPM_8_8_8_8, &btn_menu_pixmap);

    load_field_assets();

    return 0;
}


