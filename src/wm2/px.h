
// the format for my paint app
// NOTICE - original paint app is weird and doesn't save any more information,
// i wrote it like 2 years ago
// Assume 16 colours
// Also its big endian
#ifndef         WM_2_PX_H
#define         WM_2_PX_H

#include        <wm2/def.h>
#include        <wm/gdi2.h>
#include        <drivers/fs/file.h>

#define         ARGB_TO_BGRA(c) ((RGBA){.B=(c).A, .G=(c).R, .R=(c).G, .A=(c).B})

typedef struct __attribute__((__packed__))
{
        U8  PX[2]; // "PX"
        U32 W,H;
} PXHeader;

typedef RGBA PXPalette[16];

typedef struct __attribute__((__packed__))
{
        PXHeader  Header;
        PXPalette Palette;
        U8       *Image;
} PXImage;

U0       PXRender(SURFACE *Surface, PXImage *Image);
PXImage *PXLoad(SYSFILE *fp);

#endif
