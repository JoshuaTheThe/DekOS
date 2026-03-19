
#include        <wm2/wm.h>
#include        <wm2/px.h>
#include        <memory/string.h>
#include        <drivers/dev/ps2/ps2.h>
#include        <drivers/math.h>

// should probably make this a getter
extern DWORD mx, my, mbuttons, pmx, pmy;

static DispObject RootObject;
static DispObject *HoveredObject = NULL;
static SURFACE *MouseSurface = NULL;
static uint32_t *MouseSave = NULL;

static
DispObject *WM_2_HitTest(I32 x, I32 y){
        DispObject *Obj = RootObject.Children;
        DispObject *Hit = NULL;
        float       HitZ = -1.0f;
        while (Obj){
                I32 ox = Obj->PrimarySurface.X;
                I32 oy = Obj->PrimarySurface.Y;
                I32 ow = Obj->PrimarySurface.W;
                I32 oh = Obj->PrimarySurface.H;

                if (x >= ox && x < ox + ow &&
                    y >= oy && y < oy + oh){
                        /* higher Z wins */
                        if (Obj->PrimarySurface.Z > HitZ){
                                HitZ = Obj->PrimarySurface.Z;
                                Hit  = Obj;
                        }
                }
                Obj = Obj->NextSibling;
        }
        return Hit;
}

static
U0      SaveBackground(DISPLAY *Display, SURFACE *Surface, U32 *SaveBuf){
        I32 dstX = Surface->X, dstY = Surface->Y;
        I32 srcX = 0,          srcY = 0;
        I32 copyW = Surface->W, copyH = Surface->H;
        if (dstX < 0) { srcX -= dstX; copyW += dstX; dstX = 0; }
        if (dstY < 0) { srcY -= dstY; copyH += dstY; dstY = 0; }
        if (dstX + copyW > Display->W) copyW = Display->W - dstX;
        if (dstY + copyH > Display->H) copyH = Display->H - dstY;
        if (copyW <= 0 || copyH <= 0) return;
        for (I32 row = 0; row < copyH; ++row){
                const size_t SrcIdx = Display->W * (dstY + row) + dstX;
                const size_t DstIdx = Surface->W * (srcY + row) + srcX;
                memcpy(&SaveBuf[DstIdx], &Display->Framebuffer[SrcIdx],
                       copyW * (Display->BPP >> 3));
        }
}

static
U0      RestoreBackground(DISPLAY *Display, SURFACE *Surface, U32 *SaveBuf){
        I32 dstX = Surface->X, dstY = Surface->Y;
        I32 srcX = 0,          srcY = 0;
        I32 copyW = Surface->W, copyH = Surface->H;
        if (dstX < 0) { srcX -= dstX; copyW += dstX; dstX = 0; }
        if (dstY < 0) { srcY -= dstY; copyH += dstY; dstY = 0; }
        if (dstX + copyW > Display->W) copyW = Display->W - dstX;
        if (dstY + copyH > Display->H) copyH = Display->H - dstY;
        if (copyW <= 0 || copyH <= 0) return;
        for (I32 row = 0; row < copyH; ++row){
                const size_t SrcIdx = Surface->W * (srcY + row) + srcX;
                const size_t DstIdx = Display->W * (dstY + row) + dstX;
                memcpy(&Display->Framebuffer[DstIdx], &SaveBuf[SrcIdx],
                       copyW * (Display->BPP >> 3));
        }
}

static
RECT Rect = {
       .Points = {
               [0] = (POINT){ .Col = (RGBA){.B=000,    .G=000,   .R=000,   .A=255}, .X = -127.0,.Y = -127.0, .Z = 1.0 },
               [1] = (POINT){ .Col = (RGBA){.B=255,    .G=000,   .R=000,   .A=255}, .X = -127.0,.Y =  127.0, .Z = 1.0 },
               [2] = (POINT){ .Col = (RGBA){.B=000,    .G=255,   .R=000,   .A=255}, .X  = 127.0, .Y= -127.0, .Z = 1.0 },
               [3] = (POINT){ .Col = (RGBA){.B=000,    .G=000,   .R=255,   .A=255}, .X  = 127.0, .Y=  127.0, .Z = 1.0 },
        }
};

//      Redraw Window's Contents if required
__attribute__((__used__))
static
bool    WM_2_Draw(WM_2_Window *Window)
{
        size_t drawn = 0, count = 0;
        while (Window){
                if (Window->IsDirty){
                        Window->IsDirty = false;
                        GDI2ClearSurface(&Window->PrimarySurface);
                        // TODO! actual drawing here
                        GDI2DrawRect(&Window->PrimarySurface, &Rect);
                        if (Window->Children){
                                WM_2_Draw(Window->Children);
                                WM_2_Window *Child = Window->Children;
                                while (Child){
                                        GDI2BlitSurfaceToSurface(&Window->PrimarySurface,
                                                                 &Child->PrimarySurface);
                                        Child=Child->NextSibling;
                                }
                        }
                        ++drawn;
                }
                ++count;
                Window=Window->NextSibling;
        }
        return drawn > 0;
}

U0      WM_2_MoveDisplayObject(DISPLAY *Display, DispObject *Object, U32 X, U32 Y){
        RestoreBackground(Display, MouseSurface, MouseSave);
        DispObject *Obj = RootObject.Children;
        while (Obj){
                RestoreBackground(Display, &Obj->PrimarySurface, Obj->Behind);
                GDI2PartialCommit(Display, &Obj->PrimarySurface);
                Obj = Obj->NextSibling;
        }
        U32 OldX = Object->PrimarySurface.X,
            OldY = Object->PrimarySurface.Y;
        Object->PrimarySurface.X = X;
        Object->PrimarySurface.Y = Y;
        Obj = RootObject.Children;
        while (Obj){
                SaveBackground(Display, &Obj->PrimarySurface, Obj->Behind);
                GDI2BlitSurface(Display, &Obj->PrimarySurface);
                GDI2PartialCommit(Display, &Obj->PrimarySurface);
                Obj = Obj->NextSibling;
        }
        SURFACE UnionSurface = {
                .X = minu(OldX, X),
                .Y = minu(OldY, Y),
                .W = Object->PrimarySurface.W + abs((I32)X - (I32)OldX),
                .H = Object->PrimarySurface.H + abs((I32)Y - (I32)OldY),
        };
        GDI2PartialCommit(Display, &UnionSurface);
        SaveBackground(Display, MouseSurface, MouseSave);
        GDI2BlitSurface(Display, MouseSurface);
        GDI2PartialCommit(Display, MouseSurface);
}

U0      WM_2_RegisterDisplayObject(DispObject *Object){
        Object->NextSibling=RootObject.Children;
        Object->Parent=&RootObject;
        if(RootObject.Children){
                RootObject.Children->PrevSibling=Object;        
        }

        RootObject.Children=Object;
        Object->IsDirty = true;
        RootObject.IsDirty = true;
}

U0      WM_2_DeRegisterDisplayObject(DispObject *Object){
        if(RootObject.Children==Object){
                RootObject.Children=Object->NextSibling;
                if(RootObject.Children){
                        RootObject.Children->PrevSibling=NULL;
                }
                return;
        }
        
        if(Object->NextSibling){
                Object->NextSibling->PrevSibling = Object->PrevSibling;
        }if(Object->PrevSibling){
                Object->PrevSibling->NextSibling = Object->NextSibling;
        }
}

//      Background Process
U0      WM_2_PrimaryProc(U0){
        //      Load Cursor
        SYSFILE *fp = FOpen("system/cursor.px", 16, FILE_READABLE | FILE_PRESENT);
        if(!fp){
                printf(" [ERROR] Could not open cursor file\n");
                while(1);
        }

        PXImage *Image = PXLoad(fp);
        FClose(fp);
        DISPLAY *Display=NULL;
        size_t   arg_c=0;
        schedGrabArgs((char ***)&Display, &arg_c);
        if(!Display && arg_c <= 0){
                while(1)
                        ;
        }

        printf(" [INFO] Display (%x) Provided (%dx%dx%d) FRONT=%x BACK=%x\n",
                (uint32_t)Display,
                Display->W,
                Display->H,
                Display->BPP >> 3,
                (uint32_t)Display->Front,
                (uint32_t)Display->Framebuffer);
        MouseSurface = malloc(sizeof(SURFACE));
        MouseSurface->Buffer = malloc(Image->Header.W * Image->Header.H * sizeof(int));
        printf(" [INFO] Created %dx%dx%d buffer\n", Image->Header.W, Image->Header.H, sizeof(int));
        MouseSurface->DepthBuffer = NULL;
        MouseSurface->FOV = 1.0;
        MouseSurface->W = Image->Header.W;
        MouseSurface->H = Image->Header.H;
        MouseSurface->X = mx;
        MouseSurface->Y = my;
        MouseSurface->Z = 1000.0;
        MouseSurface->BPP = 32;
        PXRender(MouseSurface, Image);
        MouseSave = malloc(Image->Header.W * Image->Header.H * sizeof(uint32_t));
        
        /* Initial full draw */
        GDI2ClearDisplay(Display);
        SaveBackground(Display, MouseSurface, MouseSave);
        GDI2Commit(Display);

        for (;;){
                bool redrawn = WM_2_Draw(RootObject.Children);
                if (redrawn){
                        RestoreBackground(Display, MouseSurface, MouseSave);
                        DispObject *Obj = RootObject.Children;
                        while (Obj){
                                RestoreBackground(Display, &Obj->PrimarySurface, Obj->Behind);
                                Obj = Obj->NextSibling;
                        }

                        Obj = RootObject.Children;
                        while (Obj){
                                SaveBackground(Display, &Obj->PrimarySurface, Obj->Behind);
                                GDI2BlitSurface(Display, &Obj->PrimarySurface);
                                GDI2PartialCommit(Display, &Obj->PrimarySurface);
                                Obj = Obj->NextSibling;
                        }

                        SaveBackground(Display, MouseSurface, MouseSave);
                        GDI2BlitSurface(Display, MouseSurface);
                        GDI2PartialCommit(Display, MouseSurface);
                }

                mouseFetch((int *)&mx, (int *)&my, (int *)&pmx, (int *)&pmy, (uint8_t *)&mbuttons);
                bool mouse_moved = ((U32)MouseSurface->X != mx || (U32)MouseSurface->Y != my);
                if (mouse_moved){
                        RestoreBackground(Display, MouseSurface, MouseSave);
                        GDI2PartialCommit(Display, MouseSurface);
                        MouseSurface->X = mx;
                        MouseSurface->Y = my;
                        SaveBackground(Display, MouseSurface, MouseSave);
                        GDI2BlitSurface(Display, MouseSurface);
                        GDI2PartialCommit(Display, MouseSurface);
                }

                if (mbuttons & MOUSE_MIDDLE_BUTTON && !HoveredObject){
                        HoveredObject = WM_2_HitTest(mx, my);
                        if(HoveredObject){
                                HoveredObject->MXOffset = (I32)mx - (I32)HoveredObject->PrimarySurface.X;
                                HoveredObject->MYOffset = (I32)my - (I32)HoveredObject->PrimarySurface.Y;
                        }
                }else if (!(mbuttons & MOUSE_MIDDLE_BUTTON) && HoveredObject){
                        WM_2_MoveDisplayObject(Display,
                                               HoveredObject,
                                               (I32)mx - HoveredObject->MXOffset,
                                               (I32)my - HoveredObject->MYOffset);
                        HoveredObject = NULL;
                }
        }

        free(Image->Image);
        free(Image);
}

PROCID  WM_2_Initialise(DISPLAY *Display){
        memset(&RootObject, 0, sizeof(RootObject));
        pushf();
        cli();
        uint8_t *stack = malloc(8192*2);
        PROCID procid = schedCreateProcess(
                        "wm/2",
                        NULL,
                        0,
                        (uint8_t *)WM_2_PrimaryProc,
                        0,          /* offset == 0 */
                        stack,
                        8192*2 - 4,
                        schedGetCurrentPid(),
                        0);
        schedGetProcessN(procid)->argv = (char **)Display;
        schedGetProcessN(procid)->argc = 1;
        popf();
        return procid;
}
