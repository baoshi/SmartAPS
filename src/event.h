#ifndef EVENT_H
#define EVENT_H

/* |- parameter  -|- code -| -ID- |
 * |    HH LL     |   CC   |  DD  |
  */

#define EVENT_CONTROL_ID(X)        ((uint32_t)(X & 0x000000FF))
#define EVENT_CODE(X)              ((uint32_t)(X & 0x0000FF00) >> 8)

#define EVENT_BUTTON_DOWN           0x00000100UL
#define EVENT_BUTTON_CLICK          0x00000200UL
#define EVENT_BUTTON_HOLD_START     0x00000300UL
#define EVENT_BUTTON_HOLD_END       0x00000400UL
#define EVENT_BUTTON_PARAM(X)       ((uint32_t)(X >> 16))

#define ID_BUTTON_S1                0x00000001UL
#define ID_BUTTON_S2                0x00000002UL
#define ID_BUTTON_S3                0x00000003UL


#endif