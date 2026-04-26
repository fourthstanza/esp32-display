#include "colorfix.h"

uint16_t display_colorfix(uint16_t color)
{
    // Swap green and blue channel bitfields for this panel format
    uint16_t red = (color & 0xF800);
    uint16_t green = (color & 0x07C0) >> 6;
    uint16_t blue = (color & 0x001F) << 6;
    return red | blue | green;
}