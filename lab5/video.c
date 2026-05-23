#include <lcom/lcf.h>
#include "video.h"
#include "vbe.h"
#include <stdlib.h>

static char *video_mem;          // Virtual address to which VRAM is mapped
static vbe_mode_info_t vmi;      // VBE mode info
static unsigned bytes_per_pixel; // Number of VRAM bytes per pixel
static unsigned h_res;           // Horizontal resolution
static unsigned v_res;           // Vertical resolution
static char *back_buffer;        // Pointer to the back buffer


int vbe_set_mode(uint16_t mode) {
    reg86_t reg;
    
    // Clear the struct to avoid leaving garbage data
    memset(&reg, 0, sizeof(reg));

    reg.intno = VBE_INT;
    reg.ah = VBE_CALL;
    reg.al = VBE_SET_MODE;
    
    // Activate bit 14 to use the linear frame buffer
    reg.bx = mode | LINEAR_FRAME_BUFFER; 

    // Make the BIOS call
    if (sys_int86(&reg) != OK) {
        printf("sys_int86() failed\n");
        return 1;
    }

    // Check if the VBE function was successful
    // AX should be 0x004F (AH = 0x00 success, AL = 0x4F VBE supported)
    if (reg.ax != 0x004F) {
        printf("VBE function failed! Return code: 0x%x\n", reg.ax);
        return 1;
    }

    return 0;
}

int vbe_map_vram(uint16_t mode) {
    int r;
    struct minix_mem_range mr;
    unsigned int vram_base;
    unsigned int vram_size;

    // Get Mode Info from LCF
    if (vbe_get_mode_info(mode, &vmi) != 0) {
        printf("vbe_get_mode_info failed\n");
        return 1;
    }

    // Extract information to our global variables
    h_res = vmi.XResolution;
    v_res = vmi.YResolution;
    // Bits per pixel divided by 8 gives us bytes per pixel (rounded up)
    bytes_per_pixel = (vmi.BitsPerPixel + 7) / 8; 

    // Set Base and Size
    vram_base = vmi.PhysBasePtr;
    vram_size = h_res * v_res * bytes_per_pixel;

    // Grant memory mapping permissions
    mr.mr_base = (phys_bytes) vram_base;
    mr.mr_limit = mr.mr_base + vram_size;
    if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr))) {
        panic("sys_privctl (ADD_MEM) failed: %d\n", r);
        return 1;
    }

    // Map the memory
    video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);
    if (video_mem == MAP_FAILED) {
        panic("couldn't map video memory");
        return 1;
    }

    return 0;
}

int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color) {
    // Prevent drawing outside the screen limits (bounds checking)
    if (x >= h_res || y >= v_res) return 0;

    // Calculate the position in the 1D array
    unsigned int offset = (y * h_res + x) * bytes_per_pixel;

    // Copy only the exact number of bytes per pixel from the color variable
    memcpy(&video_mem[offset], &color, bytes_per_pixel);

    return 0;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
    for (uint16_t i = 0; i < len; i++) {
        vg_draw_pixel_bb(x + i, y, color);
    }
    return 0;
}

int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
    for (uint16_t i = 0; i < height; i++) {
        vg_draw_hline(x, y + i, width, color);
    }
    return 0;
}

int video_init_back_buffer() {
    // Tamanho total = resolução horizontal * resolução vertical * bytes por píxel
    unsigned int vram_size = h_res * v_res * bytes_per_pixel;
    
    back_buffer = (char*) malloc(vram_size);
    
    if (back_buffer == NULL) {
        printf("Failed to allocate memory for back buffer\n");
        return 1;
    }
    return 0;
}

void video_flush() {
    unsigned int vram_size = h_res * v_res * bytes_per_pixel;
    memcpy(video_mem, back_buffer, vram_size);
}

int vg_draw_pixel_bb(uint16_t x, uint16_t y, uint32_t color) {
    if (x >= h_res || y >= v_res) return 0; // Prevenção de limites

    unsigned int offset = (y * h_res + x) * bytes_per_pixel;
    memcpy(&back_buffer[offset], &color, bytes_per_pixel);
    
    return 0;
}

void video_free_back_buffer() {
    if (back_buffer != NULL) {
        free(back_buffer);
        back_buffer = NULL;
    }
}