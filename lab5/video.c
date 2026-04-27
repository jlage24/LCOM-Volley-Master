#include <lcom/lcf.h>
#include "video.h"
#include "vbe.h"

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
