
#include        <wm2/wm.h>

// should probably make this a getter
extern DWORD mx, my, mbuttons;

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
        DISPLAY *Display=NULL;
        size_t   arg_c=0;
        schedGrabArgs((char ***)&Display, &arg_c);
        if(Display && arg_c > 0){
                printf(" [INFO] Display (%x) Provided (%dx%dx%d) FRONT=%x BACK=%x\n", (uint32_t)Display, Display->W, Display->H, Display->BPP >> 3, (uint32_t)Display->Front, (uint32_t)Display->Framebuffer);
        }
        else while(1) {
                ;
        }

        SURFACE PrimarySurface = {
                        .Buffer      = malloc(256*256*sizeof(int)),
                        .DepthBuffer = NULL,
                        .FOV         = 1.0, .Z = 1.0,
                        .W           = 256,  .H = 256,
                        .X           = 256,  .Y = 256,
                        .BPP         = 32,
        };
        WM_2_Window Window = {
                .PrimarySurface = PrimarySurface,
                .IsWindow = true,
                .IsDirty  = true,
                .Parent   = NULL,
                .Children = NULL,
                .Next     = NULL,
                .Prev     = NULL
        };

        for(;;)
        {
                GDI2ClearDisplay(Display);
                WM_2_Draw(&Window);
                Window.PrimarySurface.X += 60;
                GDI2BlitSurface(Display, &Window.PrimarySurface);
                GDI2Commit(Display);
        }
}

PROCID  WM_2_Initialise(DISPLAY *Display){
        uint8_t *stack = malloc(8192);
        PROCID procid = schedCreateProcess(
                        "wm/2",
                        NULL,
                        0,
                        (uint8_t *)WM_2_PrimaryProc,
                        0,          /* offset == 0 */
                        stack,
                        8192,
                        schedGetCurrentPid(),
                        0);
        schedGetProcessN(procid)->argv = (char **)Display;
        schedGetProcessN(procid)->argc = 1;
        return procid;
}

