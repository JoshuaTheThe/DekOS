#ifndef GDI_H
#define GDI_H

#include<utils.h>
#include<resource/main.h>
#include<tty/render/render.h>

typedef struct
{
        BYTE B,G,R,A;
} RGBA;

RGBA ColourRGB(BYTE R, BYTE G, BYTE B);
DWORD ColourDword(RGBA Col);
void GDIDrawRect(DWORD X, DWORD Y, DWORD W, DWORD H);
void SetColour(RGBA Col);
RGBA GetColour(void);
void SetPixel(DWORD X, DWORD Y, RGBA Col);
void GDISetDither(DWORD X, DWORD Y, RGBA A, RGBA B, BYTE Level);
void GDIDrawLine(DWORD X1, DWORD Y1, DWORD X2, DWORD Y2);
void SetColourFn(void *(*Foo)(DWORD,DWORD));

#endif
