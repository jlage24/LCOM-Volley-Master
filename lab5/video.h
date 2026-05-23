#ifndef _LCOM_VIDEO_H_
#define _LCOM_VIDEO_H_

#include <lcom/lcf.h>
#include <stdint.h>

/**
 * @brief Switches the video card to the specified VBE graphics mode
 * @param mode VBE graphics mode to set
 * @return 0 on success, non-zero otherwise
 */
int vbe_set_mode(uint16_t mode);

/**
 * @brief Maps the physical video memory (VRAM) into the process's virtual address space
 * @param mode VBE graphics mode to get the physical address from
 * @return 0 on success, non-zero otherwise
 */
int vbe_map_vram(uint16_t mode);

/**
 * @brief Draws a pixel at the specified coordinates with a given color
 * @param x x-coordinate of the pixel
 * @param y y-coordinate of the pixel
 * @param color color to set the pixel to
 * @return 0 on success, non-zero otherwise
 */
int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Draws a horizontal line starting at the specified coordinates
 * @param x x-coordinate of the starting pixel
 * @param y y-coordinate of the starting pixel
 * @param len length of the line in pixels
 * @param color color to set the line to
 * @return 0 on success, non-zero otherwise
 */
int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);

/**
 * @brief Draws a filled rectangle starting at the specified coordinates
 * @param x x-coordinate of the top-left corner
 * @param y y-coordinate of the top-left corner
 * @param width width of the rectangle
 * @param height height of the rectangle
 * @param color color to fill the rectangle with
 * @return 0 on success, non-zero otherwise
 */
int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

/**
 * @brief Aloca a memória necessária para o back buffer
 * @return 0 em caso de sucesso, não-zero caso contrário
 */
int video_init_back_buffer();

/**
 * @brief Copia o conteúdo do back buffer para a memória de vídeo (VRAM)
 */
void video_flush();

/**
 * @brief Desenha um píxel no back buffer
 */
int vg_draw_pixel_bb(uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Liberta a memória do back buffer
 */
void video_free_back_buffer();

#endif /* _LCOM_VIDEO_H_ */
