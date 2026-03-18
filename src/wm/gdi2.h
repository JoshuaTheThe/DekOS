#ifndef GDI2_H
#define GDI2_H

#include <stdint.h>
#include <stddef.h>
#include <wm/gdi.h>

#define INFINITY (0x7ff0000000000000)

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
        RGBA Col;
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

void GDI2Pixel(SURFACE *Surface, float X, float Y, float Z, RGBA Col);
void GDI2DrawRect(SURFACE *Surface, RECT *Rect);
void GDI2DrawLine(SURFACE *Surface, POINT Rect[2]);
void GDI2BlitSurface(DISPLAY *Display, SURFACE *Surface);
void GDI2BlitSurfaceToSurface(SURFACE *Dst, SURFACE *Src);
void GDI2RenderCharacter(SURFACE *Surface, font_t *Font, char chr);
void GDI2Commit(DISPLAY *Display);
void GDI2ClearSurface(SURFACE *Surface);
void GDI2DrawRectDisplay(DISPLAY *Display, RECT *Rect);
void GDI2ClearDisplay(DISPLAY *Display);
void GDI2PartialCommit(DISPLAY *Display, SURFACE *Surface);

static inline RGBA GDI2RGBAFrom(BYTE R, BYTE G, BYTE B, BYTE A)
{
        return (RGBA){.R=R,.G=G,.B=B,.A=A};
}

static inline RGBA GDI2RGBAFromDWORD(DWORD rgba)
{
        return (RGBA){.R=rgba >> 16,.G=rgba >> 8,.B=rgba & 255,.A=rgba >> 24};
}

static inline DWORD GDI2DwordFromRGBA(RGBA rgba)
{
        return (DWORD)rgba.A << 24 | (DWORD)rgba.R << 16 | (DWORD)rgba.G << 8 | (DWORD)rgba.B;
}

#endif
