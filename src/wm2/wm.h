
#ifndef         WM_2_WM_H
#define         WM_2_WM_H

#include        <wm2/ido.h>

typedef struct{
        I32 X, Y;
        I32 W, H;
} DIRTYRECT;

U0              WM_2_PrimaryProc(U0);
PROCID          WM_2_Initialise(DISPLAY *Display);
U0              WM_2_RegisterDisplayObject(DispObject *Object);
U0              WM_2_DeRegisterDisplayObject(DispObject *Object);

#endif

