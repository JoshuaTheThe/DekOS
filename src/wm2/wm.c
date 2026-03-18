
#include        <wm2/wm.h>
#include        <wm2/px.h>
#include        <memory/string.h>

// should probably make this a getter
extern DWORD mx, my, mbuttons;

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
static
bool    WM_2_Draw(WM_2_Window *Window)
{
        if (Window->IsDirty){
                Window->IsDirty = false;
                GDI2ClearSurface(&Window->PrimarySurface);
                GDI2DrawRect(&Window->PrimarySurface, &Rect);
                return true;
        }

        return false;
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
        SURFACE *MouseSurface = malloc(sizeof(SURFACE));
        if(!MouseSurface){
                printf(" [ERROR] Could not allocate mouse surface\n");
                while(1)
                        ;
        }
        MouseSurface->Buffer = malloc(Image->Header.W * Image->Header.H * sizeof(int));
        if(!MouseSurface->Buffer){
                printf(" [ERROR] Could not allocate buffer for mouse surface\n");
                while(1)
                        ;
        }
        printf(" [INFO] Created %dx%dx%d buffer\n", Image->Header.W, Image->Header.H, sizeof(int));
        MouseSurface->DepthBuffer = NULL;
        MouseSurface->FOV = 1.0;
        MouseSurface->W = Image->Header.W;
        MouseSurface->H = Image->Header.H;
        MouseSurface->X = mx;
        MouseSurface->Y = my;
        MouseSurface->Z = 1000.0;
        MouseSurface->BPP = 32;
        WM_2_Window *Window = malloc(sizeof(WM_2_Window));
        Window->PrimarySurface.Buffer      = malloc(256*256*sizeof(int));
        Window->PrimarySurface.DepthBuffer = NULL;
        Window->PrimarySurface.FOV         = 1.0;
        Window->PrimarySurface.Z           = 1.0;
        Window->PrimarySurface.W           = 256;
        Window->PrimarySurface.H           = 256;
        Window->PrimarySurface.X           = 256;
        Window->PrimarySurface.Y           = 256;
        Window->PrimarySurface.BPP         = 32;
        Window->IsWindow = true;
        Window->IsDirty  = true;
        Window->Parent   = NULL;
        Window->Children = NULL;
        Window->Next     = NULL;
        Window->Prev     = NULL;
        PXRender(MouseSurface, Image);
        uint32_t *MouseSave = malloc(Image->Header.W * Image->Header.H * sizeof(uint32_t));
        
        /* Initial full draw */
        GDI2ClearDisplay(Display);
        WM_2_Draw(Window);
        GDI2BlitSurface(Display, &Window->PrimarySurface);
        SaveBackground(Display, MouseSurface, MouseSave);
        GDI2Commit(Display);

        for (;;){
                bool needs_commit = false;
                needs_commit |= Window->IsDirty;
                WM_2_Draw(Window);
                bool mouse_moved = ((U32)MouseSurface->X != mx || (U32)MouseSurface->Y != my);
                if (needs_commit){
                        RestoreBackground(Display, MouseSurface, MouseSave); /* undraw mouse first */
                        GDI2ClearDisplay(Display);
                        GDI2BlitSurface(Display, &Window->PrimarySurface);
                        /* re-save background at current mouse pos after window reblit */
                        SaveBackground(Display, MouseSurface, MouseSave);
                        GDI2BlitSurface(Display, MouseSurface);
                        GDI2Commit(Display);
                }

                if (mouse_moved){
                        RestoreBackground(Display, MouseSurface, MouseSave);
                        GDI2PartialCommit(Display, MouseSurface);
                        MouseSurface->X = mx;
                        MouseSurface->Y = my;
                        SaveBackground(Display, MouseSurface, MouseSave);
                        GDI2BlitSurface(Display, MouseSurface);
                        GDI2PartialCommit(Display, MouseSurface);
                }
        }

        free(Image->Image);
        free(Image);
}

PROCID  WM_2_Initialise(DISPLAY *Display){
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
