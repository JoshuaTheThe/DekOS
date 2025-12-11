#ifndef WM_H
#define WM_H

#include<utils.h>
#include<resource/main.h>
#include<tty/render/render.h>
#include<memory/alloc.h>
#include<memory/string.h>
#include<wm/gdi.h>
#include<tty/input/input.h>
#include<drivers/math.h>

#define WINDOW_POSITION_DEFAULT_X (64)
#define WINDOW_POSITION_DEFAULT_Y (64)

#define MAX_TITLE_LENGTH (32)
#define MAX_SEGMENTS (1024)

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
                } Text;
        } ElementData;
} ELEMENT;

typedef struct
{
        DWORD X,Y,PX,PY;
        DWORD START_X, START_Y; /* For Actions */
        DWORD W,H;
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

KRNLRES *WMCreateWindow(char *Title, DWORD X, DWORD Y, DWORD W, DWORD H);
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
