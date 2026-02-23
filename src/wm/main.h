#ifndef WM_H
#define WM_H

#include<utils.h>
#include<resource/main.h>
#include<tty/render/render.h>
#include<memory/alloc.h>
#include<memory/string.h>
#include<wm/gdi.h>
#include <drivers/dev/ps2/ps2.h>
#include<drivers/math.h>

#define WINDOW_POSITION_DEFAULT_X (64)
#define WINDOW_POSITION_DEFAULT_Y (64)

#define WINDOW_WIDTH_DEFAULT (640)
#define WINDOW_HEIGHT_DEFAULT (350)

#define WINDOW_PADDING_DEFAULT (4)
#define WINDOW_THICKNESS_DEFAULT (1)
#define WINDOW_INNER_DEFAULT (ColourRGB(0xF0,0xF0,0xF0))
#define WINDOW_OUTER_DEFAULT (ColourRGB(0xC0,0xC0,0xC0))
#define WINDOW_BORDER_DEFAULT (ColourRGB(0x00,0x00,0x00))
#define WINDOW_TITLEBAR_HEIGHT_DEFAULT (4)

#define MAX_TITLE_LENGTH (32)
#define MAX_SEGMENTS (64)

typedef enum
{
        WINDOW_ELEMENT_NONE,
        WINDOW_ELEMENT_BITMAP,
        WINDOW_ELEMENT_TEXT,
} ELEMENTTYPE;

typedef enum
{
        WINDOW_NORMAL,
        WINDOW_MINIMISED,
        WINDOW_MAXIMISED,
        WINDOW_CLOSED,
} WINSTATE;

/**
 * These are made by creating a Kernel Resource,
 * with the window as the parent.
 */
typedef struct
{
        DWORD X, Y, W, H;
        ELEMENTTYPE EType;
        union
        {
                struct
                {
                        DWORD *Buff;
                        DWORD  W,H;
                } Bitmap;

                struct
                {
                        char **Text;
                        DWORD Lines;
                        DWORD Columns;
                        font_t *Font;

                        RGBA Bg, Fg;
                } Text;
        } ElementData;
} ELEMENT;

typedef struct
{
        DWORD X,Y,PX,PY;
        DWORD START_X, START_Y; /* For Actions */
        DWORD W,H,Padding,Thickness,TitleBarHeight;
        RGBA Outer, Inner, Border;
        char  Title[MAX_TITLE_LENGTH];
        BOOL  RequiresRedraw;
        BOOL  CanMove, InAction;
        WINSTATE State;
} WINDOW;

typedef struct
{
        DWORD X, Y, EX, EY;
        BOOL Dirty;
} SEGMENT;

KRNLRES *WMCreateWindow(char *Title, DWORD X, DWORD Y, DWORD W, DWORD H, DWORD Padding, DWORD Thickness, DWORD TitleBarHeight, RGBA Outer, RGBA Inner, RGBA Border);
void WMIterate(void);
PROCID WMInit(void);
void WMMove(WINDOW *Window, DWORD X, DWORD Y);
void WMAction(KRNLRES *P);
void WMDraw(KRNLRES *P);
KRNLRES *WMCreateElement(KRNLRES *Window, DWORD X, DWORD Y, DWORD W, DWORD H, ELEMENTTYPE Type);
DWORD WMMiddlePointX(WINDOW *Window);
DWORD WMMiddlePointY(WINDOW *Window);
char WMGetChar(KRNLRES *Window);
DWORD TitleBarHeight(WINDOW *Window);
BOOL WMIsFocused(KRNLRES *Window);

#endif
