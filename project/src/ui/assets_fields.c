#include "assets.h"

#pragma clang optimize off

#include "../images/beach_volley.xpm"
#include "../images/desert_volley.xpm"
#include "../images/jungle_volley.xpm"
#include "../images/ice_volley.xpm"


void load_field_assets() {
    xpm_load((xpm_map_t) beach_field_xpm, XPM_8_8_8_8, &beach_volley_pixmap);
    xpm_load((xpm_map_t) desert_field_xpm, XPM_8_8_8_8, &desert_volley_pixmap);
    xpm_load((xpm_map_t) jungle_field_xpm, XPM_8_8_8_8, &jungle_volley_pixmap);
    xpm_load((xpm_map_t) ice_field_xpm, XPM_8_8_8_8, &ice_volley_pixmap);
}

#pragma clang optimize on
