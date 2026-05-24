#include "assets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t *bg_pixmap = NULL;

int load_all_assets() {
    // 1. Alocar a memória EXATA: 1024 * 768 píxeis * 3 bytes (24-bits)
    bg_pixmap = malloc(1024 * 768 * 3);
    if (bg_pixmap == NULL) return 1;

    FILE *f = fopen("/home/lcom/labs/project/src/images/fundo.bmp", "rb");
    if (f == NULL) return 1;

    fseek(f, 54, SEEK_SET); // Saltar cabeçalho
    uint8_t row[1024 * 3];

    for (int y = 0; y < 768; y++) {
        fread(row, 1, 1024 * 3, f);
        memcpy(&bg_pixmap[y * 1024 * 3], row, 1024 * 3);
    }
    fclose(f);
    return 0;
}

