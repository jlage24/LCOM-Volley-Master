#ifndef _LCOM_VIDEO_H_
#define _LCOM_VIDEO_H_

#include <lcom/lcf.h>
#include <stdint.h>

/**
 * @brief Switches the video card to the specified VBE graphics mode
 * * @param mode VBE graphics mode to set
 * @return 0 on success, non-zero otherwise
 */
int vbe_set_mode(uint16_t mode);

#endif /* _LCOM_VIDEO_H_ */
