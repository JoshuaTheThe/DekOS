#ifndef WM_H
#define WM_H

#include<utils.h>
#include<resource/main.h>
#include<tty/render/render.h>
#include<memory/alloc.h>
#include<memory/string.h>
#include<wm/gdi.h>
#include<tty/input/input.h>

#define WINDOW_POSITION_DEFAULT_X (64)
#define WINDOW_POSITION_DEFAULT_Y (64)

#define MAX_TITLE_LENGTH (32)
#define MAX_SEGMENTS (1024)

typedef struct
{
        DWORD X,Y,PX,PY;
        DWORD START_X, START_Y; /* For Actions */
        DWORD W,H;
        char  Title[MAX_TITLE_LENGTH];
        BOOL  RequiresRedraw;
        BOOL  CanMove, InAction;
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

#endif
