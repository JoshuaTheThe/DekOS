#ifndef GDI2_H
#define GDI2_H

#include <stdint.h>
#include <stddef.h>

typedef struct
{
        uint8_t R, G, B, A;
} ColourRGBA;

typedef struct
{
        uint32_t *Framebuffer;
        uint32_t *Front;
        float *DepthBuffer;
        float FOV;
        uint16_t W, H;
        uint8_t BPP;
} DISPLAY;

typedef struct
{
        ColourRGBA Col;
        float X, Y, Z;
} POINT;

typedef struct
{
        POINT Points[4];
} RECT;

typedef struct
{
        uint32_t *Buffer;
        float *DepthBuffer;
        float FOV, Z;
        int W, H, X, Y;
        uint8_t BPP;
} SURFACE;

void GDI2Pixel(SURFACE *Surface, float X, float Y, float Z, ColourRGBA Col);
void GDI2DrawRect(SURFACE *Surface, RECT *Rect);
void GDI2BlitSurface(DISPLAY *Display, SURFACE *Surface);

#endif
