/**
 * TODO - Move To a user program
 */

#include <wm/main.h>
#include <tty/output.h>

extern KRNLRES grResources;
extern DWORD mx, my, mbuttons;

KRNLRES *Windows;

KRNLRES *FocusedWindow = NULL;

SEGMENT Segments[MAX_SEGMENTS];

DWORD MovementX, MovementY;

DWORD Thickness = 1;
DWORD Padding = 4;
DWORD InternalPadding = 4 + 1;
DWORD ElementPadding = 2;
DWORD TextPaddingY = 2;

DWORD TitleBarHeight(WINDOW *Window)
{
        if (!Window)
                return TextPaddingY * 2 + font_8x8.char_height + Thickness;
        return TextPaddingY * 2 + font_8x8.char_height + Thickness;
}

/**
 * Find the segment for a given XY Position.
 */
SEGMENT *FindSegment(DWORD X, DWORD Y)
{
        int i;
        for (i = 0; i < MAX_SEGMENTS; ++i)
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
        RESULT Result;
        KRNLRES *Window = ResourceCreateK(Windows, RESOURCE_TYPE_WINDOW,
                                          sizeof(WINDOW), schedGetCurrentPid(),
                                          &Result);
        // printf("Result from window creation: %d\n", Result);
        if (!Window)
                return (KRNLRES *)NULL;
        ((WINDOW *)Window->Region.ptr)->X = X;
        ((WINDOW *)Window->Region.ptr)->Y = Y;
        ((WINDOW *)Window->Region.ptr)->PX = 0;
        ((WINDOW *)Window->Region.ptr)->PY = 0;
        ((WINDOW *)Window->Region.ptr)->W = W;
        ((WINDOW *)Window->Region.ptr)->H = H;
        strncpy(((WINDOW *)Window->Region.ptr)->Title, Title, MAX_TITLE_LENGTH);
        ((WINDOW *)Window->Region.ptr)->Title[MAX_TITLE_LENGTH - 1] = 0;
        ((WINDOW *)Window->Region.ptr)->RequiresRedraw = TRUE;
        ((WINDOW *)Window->Region.ptr)->CanMove = TRUE;
        ((WINDOW *)Window->Region.ptr)->START_X = 0;
        ((WINDOW *)Window->Region.ptr)->START_Y = 0;
        ((WINDOW *)Window->Region.ptr)->InAction = FALSE;
        ((WINDOW *)Window->Region.ptr)->State = WINDOW_NORMAL;
        return Window;
}

KRNLRES *WMCreateElement(KRNLRES *Window, DWORD X, DWORD Y, DWORD W, DWORD H, ELEMENTTYPE Type)
{
        RESULT Result;
        if (!Window || Window->Type != RESOURCE_TYPE_WINDOW)
        {
                return (KRNLRES *)NULL;
        }
        KRNLRES *Element = ResourceCreateK(Window, RESOURCE_TYPE_ELEMENT,
                                           sizeof(ELEMENT), schedGetCurrentPid(),
                                           &Result);
        // printf("Result from Element creation: %d\n", Result);
        if (!Element)
                return (KRNLRES *)NULL;
        ((ELEMENT *)Element->Region.ptr)->X = X;
        ((ELEMENT *)Element->Region.ptr)->Y = Y;
        ((ELEMENT *)Element->Region.ptr)->W = W;
        ((ELEMENT *)Element->Region.ptr)->H = H;
        ((ELEMENT *)Element->Region.ptr)->EType = Type;
        ((WINDOW *)Window->Region.ptr)->RequiresRedraw = TRUE;
        return Element;
}

/**
 * WMBackgroundPattern, Generate the pixel
 * for a given XY Co-Ordinate in the background.
 */
void WMBackgroundPattern(DWORD X, DWORD Y)
{
        static RGBA LightB = {0};
        static RGBA DarkB = {0};
        if (LightB.A == 0)
        {
                LightB = ColourRGB(0x00, 0x00, 0xFF);
                DarkB = ColourRGB(0x00, 0x00, 0x7F);
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
        RenderGetDim((int *)Dim);

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
        static RGBA DarkB = {0};
        if (LightB.A == 0)
        {
                LightB = ColourRGB(0x40, 0x00, 0xFF);
                DarkB = ColourRGB(0x40, 0x00, 0x7F);
        }
        GDISetDither(X, Y, LightB, DarkB, X / 2);
}

/**
 * Draw a given element.
 */
void WMDrawElement(WINDOW *Window, ELEMENT *Element)
{
        if (!Window || !Element)
                return;

        SIZE X = (Thickness + InternalPadding) + Element->X;
        SIZE Y = (TitleBarHeight(Window) + InternalPadding + Thickness) + Element->Y;
        SIZE MX = X + (Window->W - (Thickness + InternalPadding) * 2) + 1;
        SIZE MY = Y + (Window->H - TitleBarHeight(Window) - (Thickness + InternalPadding) * 2);
        SIZE BW = MX - X;
        SIZE BH = MY - Y;
        SIZE W = min(Element->W, BW);
        SIZE H = min(Element->H, BH);
        X = min(X, W) + Window->X;
        Y = min(Y, H) + Window->Y;

        // printf("Drawing Element of type %d\n", Element->EType);

        if (Element->EType == WINDOW_ELEMENT_TEXT &&
            Element->ElementData.Text.Font &&
            Element->ElementData.Text.Text)
        {
                GDIBorderedRect(ColourRGB(0xC0, 0xC0, 0xC0),
                                ColourRGB(0xF0, 0xF0, 0xF0),
                                ColourRGB(0, 0, 0),
                                X, Y, W, H,
                                ElementPadding,
                                Thickness);

                SIZE TextX = X + ElementPadding + Thickness;
                SIZE TextY = Y + ElementPadding + Thickness;

                char **TextLines = Element->ElementData.Text.Text;
                DWORD LineCount = Element->ElementData.Text.Lines;
                DWORD Columns = Element->ElementData.Text.Columns;
                font_t *Font = Element->ElementData.Text.Font;

                RenderSetFont(Font);

                DWORD TextColor = rgb(0x00, 0x00, 0x00);
                DWORD BackgroundColor = rgb(0xF0, 0xF0, 0xF0);

                DWORD AvailableWidth = W - (ElementPadding + Thickness) * 2;
                DWORD AvailableHeight = H - (ElementPadding + Thickness) * 2;
                DWORD MaxLines = AvailableHeight / Font->char_height;
                DWORD MaxCharsPerLine = AvailableWidth / Font->char_width;

                cli();
                for (DWORD line = 0; line < LineCount && line < MaxLines; line++)
                {
                        if (TextLines[line] == NULL)
                                continue;

                        DWORD CurrentY = TextY + (line * Font->char_height);
                        for (DWORD col = 0; col < Columns && col < MaxCharsPerLine; col++)
                        {
                                char ch = TextLines[line][col];
                                if (ch == '\0')
                                        break;

                                DWORD CurrentX = TextX + (col * Font->char_width);
                                RenderChar(ch, CurrentX, CurrentY, BackgroundColor, TextColor);
                        }

                        if (CurrentY + Font->char_height > TextY + AvailableHeight)
                                break;
                }
                sti();
        }
        else if (Element->EType == WINDOW_ELEMENT_BITMAP &&
                 Element->ElementData.Bitmap.Buff)
        {
                // printf("Bitmap element rendering not implemented\n");
        }
}

/**
 * Draw a given window.
 */
void WMDraw(KRNLRES *P)
{
        font_t *font = RenderGetFont();
        if (!P || !P->Region.ptr)
                return;
        if (P->Type != RESOURCE_TYPE_WINDOW)
        {
                // printf("Tried to render non-window as window\n");
                return;
        }
        WINDOW *Window = (WINDOW *)P->Region.ptr;
        if (!Window)
        {
                // printf("Window Information does not exist (\?\?)\n");
                return;
        }
        if (!Window->RequiresRedraw)
                return;

        /**
         * Overwrite Previous
         */
        if (Window->PX == Window->X && Window->PY == Window->Y)
                goto OnlyDrawElements;
        for (DWORD Yo = 0; Yo < Window->H; ++Yo)
                for (DWORD Xo = 0; Xo < Window->W; ++Xo)
                {
                        WMBackgroundPattern(Xo + Window->PX, Yo + Window->PY);
                }

        /**
         * Border
         */
        GDIBorderedRect(ColourRGB(0xC0, 0xC0, 0xC0),
                        ColourRGB(0xF0, 0xF0, 0xF0),
                        ColourRGB(0, 0, 0),
                        Window->X,
                        Window->Y,
                        Window->W,
                        Window->H,
                        Padding,
                        Thickness);

        /**
         * TitleBar And Icons (e.g. Close)
         */
        /**
         * Title
         */
        RenderSetFont(&font_8x8);
        DWORD X = (Window->W - strnlen(Window->Title, MAX_TEXT_LENGTH) * font_8x8.char_width - (Thickness + Padding)) / 2;
        DWORD Y = Thickness + Padding + TextPaddingY;
        SetColourFn(WMTitleBar);
        GDIDrawRect(Window->X + Padding, Window->Y + Thickness + Padding, Window->W - Padding * 2, TitleBarHeight(Window));
        SetColour(ColourRGB(0, 0, 0));
        RenderPrint((unsigned char *)Window->Title, X + Window->X, Y + Window->Y, rgb(0xf0, 0xf0, 0xf0), rgb(0, 0, 0));
        RenderSetFont(font);
        GDIDrawRect(Window->X + Padding, Window->Y + Thickness + Padding + TextPaddingY * 2 + font_8x8.char_height, Window->W - Padding * 2, Thickness);
        /**
         * ToolBar
         */

        /**
         * Draw Elements
         */
OnlyDrawElements:
        (void)0;
        KRNLRES *Element = P->FirstChild;
        while (Element)
        {
                if (Element->Type == RESOURCE_TYPE_ELEMENT)
                {
                        WMDrawElement(Window, Element->Region.ptr);
                }
                Element = Element->NextSibling;
        }

        /* Finally State that we do not need to redraw */
        Window->RequiresRedraw = FALSE;
        Window->CanMove = TRUE;
        Window->PX = Window->X;
        Window->PY = Window->Y;
}

/**
 * The several Window actions, e.g. move
 */
void WMAction(KRNLRES *P)
{
        if (!P || !P->Region.ptr)
                return;
        if (P->Type != RESOURCE_TYPE_WINDOW)
        {
                // printf("Tried to render non-window as window\n");
                return;
        }
        WINDOW *Window = (WINDOW *)P->Region.ptr;
        if (!Window)
        {
                // printf("Window Information does not exist (\?\?)\n");
                return;
        }

        if (mbuttons & MOUSE_LEFT_BUTTON && !Window->InAction)
        {
                Window->START_X = mx;
                Window->START_Y = my;
                Window->InAction = TRUE;
                FocusedWindow = P;
                Window->RequiresRedraw = TRUE;
        }
        if (Window->InAction && !(mbuttons & MOUSE_LEFT_BUTTON))
        {
                Window->InAction = FALSE;
                DWORD TH = TitleBarHeight(Window) + Padding;
                DWORD PosX = Window->START_X - Window->X;
                DWORD PosY = Window->START_Y - Window->Y;
                BOOL InTitleBar = (PosX <= Window->W) && (PosY <= TH);
                if (InTitleBar)
                {
                        WMMove(Window, mx, my);
                        return;
                }
                /**
                 * Otherwise, Invoke user, let them figure it out.
                 */
        }
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
                if (P != FocusedWindow)
                {
                        WMDraw(P);
                        WMAction(P);
                }
        } while (ResourceNextOfType(&P, RESOURCE_TYPE_WINDOW));
        if (FocusedWindow)
        {
                WMDraw(FocusedWindow);
                WMAction(FocusedWindow);
        }
}

/**
 * Move A Window.
 */
void WMMove(WINDOW *Window, DWORD X, DWORD Y)
{
        if (!Window->CanMove)
                return;
        Window->PX = Window->X;
        Window->PY = Window->Y;
        Window->X = X;
        Window->Y = Y;
        Window->RequiresRedraw = TRUE;
        Window->CanMove = FALSE;
}

/**
 * Where the WM process starts.
 */
void WMMain(void)
{
        while (!Windows)
                ;
        WMDrawBackground();
        while (true)
        {
                WMIterate();
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
        return schedCreateProcess("wm", NULL, 0, (uint8_t *)WMMain, 0, stack, 8192, schedGetKernelPid(), 0);
}

/**
 * Return the middle of the body (X)
 */
DWORD WMMiddlePointX(WINDOW *Window)
{
        if (!Window)
                return 0;

        SIZE X = Window->X + Thickness + InternalPadding;
        SIZE MX = X + (Window->W - (Thickness + InternalPadding) * 2) + 1;
        SIZE BW = MX - X;
        return BW / 2;
}

/**
 * Return the middle of the body (Y)
 */
DWORD WMMiddlePointY(WINDOW *Window)
{
        if (!Window)
                return 0;

        SIZE Y = Window->Y + TitleBarHeight(Window) + InternalPadding + Thickness;
        SIZE MY = Y + (Window->H - TitleBarHeight(Window) - (Thickness + InternalPadding) * 2);
        SIZE BH = MY - Y;
        return BH / 2;
}

BOOL WMIsFocused(KRNLRES *Window)
{
        if (!FocusedWindow)
                return FALSE;
        return FocusedWindow == Window;
}

// /**
//  * For User
//  */
// char WMGetChar(KRNLRES *Window)
// {
//         if (!Window || Window->Type != RESOURCE_TYPE_WINDOW)
//                 return 0;
// 
//         WINDOW *target_window = (WINDOW *)Window->Region.ptr;
//         if (!target_window)
//                 return 0;
// 
//         while (true)
//         {
//                 if (FocusedWindow == Window)
//                 {
//                         return getchar();
//                 }
//                 else
//                 {
//                         if (target_window->State == WINDOW_CLOSED)
//                                 return 0;
//                 }
//         }
// 
//         return 0;
// }