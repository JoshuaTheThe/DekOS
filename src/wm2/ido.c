
#include        <wm2/wm.h>

DispObject     *CreateDisplayObject(U32 X, U32 Y, U32 W, U32 H, U8 BPP){
        DispObject *Object = malloc(sizeof(DispObject));
        Object->Behind                = malloc(W * H * (BPP >> 3));
        Object->PrimarySurface.Buffer = malloc(W * H * (BPP >> 3));
        Object->PrimarySurface.W      = W;
        Object->PrimarySurface.H      = H;
        Object->PrimarySurface.X      = X;
        Object->PrimarySurface.Y      = Y;
        Object->PrimarySurface.BPP    = BPP;
        Object->PrimarySurface.FOV    = 1.0;
        Object->PrimarySurface.DepthBuffer = NULL;
        Object->PrevSibling =
        Object->NextSibling =
        Object->Parent      =
        Object->Children    = NULL;
        Object->IsDirty     = false;
        Object->IsWindow    = false;
        return      Object;
}
