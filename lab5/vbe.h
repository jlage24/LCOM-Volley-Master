#ifndef _LCOM_VBE_H_
#define _LCOM_VBE_H_

#include <lcom/lcf.h>

/* VBE Interrupt */
#define VBE_INT               0x10

/* VBE AH Register Commands */
#define VBE_CALL              0x4F

/* VBE Function Codes (AL Register) */
#define VBE_GET_CTRL_INFO     0x00
#define VBE_GET_MODE_INFO     0x01
#define VBE_SET_MODE          0x02

/* VBE Mode Bitmask */
#define LINEAR_FRAME_BUFFER   BIT(14)

/* VBE Return Values */
#define VBE_SUCCESS           0x00
#define VBE_FUNC_CALL_FAILED  0x01
#define VBE_FUNC_NOT_SUPPORT  0x02
#define VBE_FUNC_INVALID      0x03

#endif /* _LCOM_VBE_H_ */
