
/**
 * Interactable Displayed Object - A simple architecture for abstract display objects, such as
 * 	- windows
 * 	- elements
 * 	- other ui features.
 */

#ifndef         WM2_IDO_H
#define         WM2_IDO_H

#include        <wm/gdi2.h>
#include        <wm2/def.h>

typedef struct InDispObj {
        SURFACE PrimarySurface;
        I32     MXOffset,MYOffset;
        U32    *Behind;
        float  *BehindZ;
        bool    IsWindow,IsDirty;
        struct  InDispObj
                *Parent,
                *Children,
                *NextSibling,*PrevSibling;
        bool  (*OnDraw)(struct InDispObj *Self);
        void  (*OnClick)(struct InDispObj *Self, I32 x, I32 y, U32 buttons);
        void  (*OnMove)(struct InDispObj *Self, I32 x, I32 y);
}       InDispObj;

typedef InDispObj DispObject;
typedef struct InDispObj WM_2_Window;

void    DestroyDisplayObject(DispObject *Object);
DispObject     *CreateDisplayObject(U32 X, U32 Y, U32 W, U32 H, U8 BPP);
        
#endif

