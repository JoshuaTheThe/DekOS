#include <wm/main.h>
#include <tty/output/output.h>

extern KRNLRES grResources;
KRNLRES *Windows;

SEGMENT Segments[MAX_SEGMENTS];

/**
 * Find the segment for a given XY Position.
 */
SEGMENT *FindSegment(DWORD X, DWORD Y)
{
        int i;
        for (i=0; i < MAX_SEGMENTS; ++i)
        {
                if ((X >= Segments[i].X && X < Segments[i].EX) &&
                    (Y >= Segments[i].Y && Y < Segments[i].EY))
                {
                        return &Segments[i];
                }
        }

        return NULL;
}

/**
 * WMCreateWindow, just a helper constructor function
 */
KRNLRES *WMCreateWindow(char *Title, DWORD X, DWORD Y, DWORD W, DWORD H)
{
        RESULT result;
        KRNLRES *Window = ResourceCreateK(Windows, RESOURCE_TYPE_WINDOW,
                                          sizeof(WINDOW), schedGetCurrentPid(),
                                          &result);
        printf("Result from window creation: %d\n", result);
        if (!Window)
                return (KRNLRES *)NULL;
        ((WINDOW *)Window->Region.ptr)->X = X;
        ((WINDOW *)Window->Region.ptr)->Y = Y;
        ((WINDOW *)Window->Region.ptr)->W = W;
        ((WINDOW *)Window->Region.ptr)->H = H;
        strncpy(((WINDOW *)Window->Region.ptr)->Title, Title, MAX_TITLE_LENGTH);
        ((WINDOW *)Window->Region.ptr)->Title[MAX_TITLE_LENGTH-1] = 0;
        return Window;
}

void WMBackgroundPattern(DWORD X, DWORD Y)
{
        static RGBA LightB = {0};
        static RGBA DarkB  = {0};
        if (LightB.A == 0)
        {
                LightB = ColourRGB(0x00, 0x00, 0xFF);
                DarkB  = ColourRGB(0x00, 0x00, 0x7F);
        }
        GDISetDither(X, Y, LightB, DarkB, X / 10);
        SetPixel(X, Y, GetColour());
}

/**
 * Draw the WM's Background image.
 * In this case, an alternating pattern.
 * Only inefficient for complete redraw,
 * I wish to implement it so that it only redraws,
 * what is needed.
 */
void WMDrawBackground(void)
{
        DWORD Dim[3];
        RenderGetDim(Dim);

        for (DWORD y = 0; y < Dim[1]; y += 1)
        {
                for (DWORD x = 0; x < Dim[0]; x += 1)
                {
                        WMBackgroundPattern(x, y);
                }
        }
}

/**
 * ...
 */
void WMTitleBar(DWORD X, DWORD Y)
{
        static RGBA LightB = {0};
        static RGBA DarkB  = {0};
        if (LightB.A == 0)
        {
                LightB = ColourRGB(0x40, 0x00, 0xFF);
                DarkB  = ColourRGB(0x40, 0x00, 0x7F);
        }
        GDISetDither(X, Y, LightB, DarkB, X / 2);
}

/**
 * Draw a given window.
 */
void WMDraw(KRNLRES *P)
{
        if (!P || !P->Region.ptr)
                return;
        if (P->Type != RESOURCE_TYPE_WINDOW)
        {
                printf("Tried to render non-window as window\n");
                return;
        }
        WINDOW *Window = (WINDOW *)P->Region.ptr;
        if (!Window)
        {
                printf("Window Information does not exist (??)\n");
                return;
        }

        DWORD Thickness = 1;
        DWORD Padding = 4;
        DWORD TextPaddingY = 2;

        /**
         * Border
         */
        SetColour(ColourRGB(0xC0, 0xC0, 0xC0));
        GDIDrawRect(Window->X, Window->Y, Window->W, Window->H);
        SetColour(ColourRGB(0xF0, 0xF0, 0xF0));
        GDIDrawRect(Window->X+Padding+Thickness, Window->Y+Padding+Thickness, Window->W-(Padding+Thickness)*2, Window->H-(Padding+Thickness)*2);
        SetColour(ColourRGB(0, 0, 0));
        /* Outer */
        /* Top */
        GDIDrawRect(Window->X, Window->Y, Window->W, Thickness);
        /* Bottom */
        GDIDrawRect(Window->X, Window->Y+Window->H, Window->W, Thickness);
        /* Left */
        GDIDrawRect(Window->X, Window->Y, Thickness, Window->H);
        /* Right */
        GDIDrawRect(Window->X+Window->W, Window->Y, Thickness, Window->H + Thickness);
        /* Inner */
        /* Top */
        GDIDrawRect(Window->X + Padding, Window->Y + Padding, Window->W - Padding*2, Thickness);
        /* Bottom */
        GDIDrawRect(Window->X + Padding, Window->Y+Window->H-Padding, Window->W - Padding*2, Thickness);
        /* Left */
        GDIDrawRect(Window->X + Padding, Window->Y + Padding, Thickness, Window->H - Padding*2);
        /* Right */
        GDIDrawRect(Window->X+Window->W - Padding, Window->Y + Padding, Thickness, Window->H + Thickness - Padding*2);
        /**
         * TitleBar And Icons (e.g. Close)
         */
        /**
         * Title
         */
        font_t *font = RenderGetFont();
        RenderSetFont(&font_8x8);
        DWORD X = (Window->W-strnlen(Window->Title, MAX_TEXT_LENGTH)*font_8x8.char_width-(Thickness+Padding))/2;
        DWORD Y = Thickness+Padding+TextPaddingY;
        SetColourFn(WMTitleBar);
        GDIDrawRect(Window->X + Padding, Window->Y + Thickness+Padding, Window->W - Padding*2, TextPaddingY*2+font_8x8.char_height);
        SetColour(ColourRGB(0, 0, 0));
        RenderPrint(Window->Title, X+Window->X, Y+Window->Y, rgb(0xf0, 0xf0, 0xf0), rgb(0, 0, 0));
        RenderSetFont(font);
        GDIDrawRect(Window->X + Padding, Window->Y + Thickness+Padding+TextPaddingY*2+font_8x8.char_height, Window->W - Padding*2, Thickness);
        /**
         * ToolBar
         */
}

/**
 * Draw the entire scene.
 */
void WMIterate(void)
{
        KRNLRES *P = Windows->FirstChild;
        if (!P)
                return;
        do
        {
                printf("Window, of (%d,%d,%d,%d) At %x\n", ((WINDOW *)P->Region.ptr)->X, ((WINDOW *)P->Region.ptr)->Y, ((WINDOW *)P->Region.ptr)->W, ((WINDOW *)P->Region.ptr)->H, P);
                WMDraw(P);
        }
        while (ResourceNextOfType(&P, RESOURCE_TYPE_WINDOW));
}

/**
 * Where the WM process starts.
 */
void WMMain(void)
{
        while (!Windows)
                ;
        WMDrawBackground();
        WMIterate();
        while (true)
        {
                /**
                 * WMTick()
                 */
        }
}

/**
 * Create the WM Process.
 */
PROCID WMInit(void)
{
        uint8_t *stack = malloc(8192);
        memset(Segments, 0, sizeof(Segments));
        Windows = ResourceCreateK(&grResources, RESOURCE_TYPE_RAW, 0, schedGetKernelPid(), NULL);
        return schedCreateProcess("wm", NULL, 0, (uint8_t *)WMMain, 0, stack, 8192, schedGetKernelPid());
}
