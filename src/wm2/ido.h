
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
        bool    IsWindow,IsDirty;
        struct  InDispObj
                *Parent,
                *Children,
                *Next,*Prev;
}       InDispObj;

typedef InDispObj DispObject;
typedef struct InDispObj WM_2_Window;

#endif

