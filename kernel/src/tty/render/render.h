#ifndef TEXT_H
#define TEXT_H

#include <utils.h>
#include <resource/main.h>
#include <tty/render/fonts.h>

#define MAX_TEXT_LENGTH 1024

typedef enum alignType_t
{
        ALIGN_CENTER,
        ALIGN_LEFT_TOP, /* LEFT OR TOP */
        ALIGN_RIGHT_BOTTOM, /* RIGHT OR BOTTOM */

        ALIGN_TOP=ALIGN_LEFT_TOP,
        ALIGN_LEFT=ALIGN_LEFT_TOP,
        ALIGN_BOTTOM=ALIGN_RIGHT_BOTTOM,
        ALIGN_RIGHT=ALIGN_RIGHT_BOTTOM
} alignType_t;

/* Getters && Setters */
void RenderSetFont(font_t *fNewFont);
font_t *RenderGetFont(void);
void RenderGetDim(int iDim[3]);
void RenderSetDim(int iNewDim[3], BOOL bAffected[3]);

/* functions */
uint32_t RenderBlend(uint32_t bg, uint32_t fg, uint8_t intensity, uint8_t bpp);
void RenderChar(unsigned char chr, uint32_t px, uint32_t py, uint32_t bg, uint32_t fg);
void RenderPrint(const unsigned char *text, uint32_t px, uint32_t py, uint32_t bg, uint32_t fg);
void RenderAlign(const char *text, uint32_t *px, uint32_t *py, int marginX, int marginY,
           alignType_t alignX, alignType_t alignY);
void RenderCenter(const char *text, uint32_t *px, uint32_t *py);

extern RID rdFrameRID;

#endif
