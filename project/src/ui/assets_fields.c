/*
 * Field background asset loading.
 *
 * Field XPMs are large, so they are loaded from a separate file while still
 * filling the global pixmaps declared in assets.h and consumed by menu.c.
 */
#include "assets.h"

#pragma clang optimize off

#include "../images/fields/beach_volley.xpm"
#include "../images/fields/desert_volley.xpm"
#include "../images/fields/jungle_volley.xpm"
#include "../images/fields/ice_volley.xpm"


void load_field_assets() {
    /* These backgrounds are optional variants selected by game.c before rendering. */
    xpm_load((xpm_map_t) beach_field_xpm, XPM_8_8_8_8, &beach_volley_pixmap);
    xpm_load((xpm_map_t) desert_field_xpm, XPM_8_8_8_8, &desert_volley_pixmap);
    xpm_load((xpm_map_t) jungle_field_xpm, XPM_8_8_8_8, &jungle_volley_pixmap);
    xpm_load((xpm_map_t) ice_field_xpm, XPM_8_8_8_8, &ice_volley_pixmap);
}

#pragma clang optimize on
