#include <wm/gdi2.h>
#include <drivers/dev/serial.h>
#include <string.h>
#include <drivers/math.h>
#include <wm2/def.h>

void GDI2Pixel32(SURFACE *Surface, float X, float Y, float Z, RGBA Col)
{
        if (!Surface || !Surface->Buffer || Z <= 0.1)
                return;
        int64_t SX = (int64_t)(Surface->W / 2 + (X / Z) * Surface->FOV);
        int64_t SY = (int64_t)(Surface->H / 2 - (Y / Z) * Surface->FOV);
        size_t Index = Surface->W * SY + SX;
        if (SX >= Surface->W || SY >= Surface->H || SX < 0 || SY < 0)
                return;
        if (!Surface->DepthBuffer || (Surface->DepthBuffer && Surface->DepthBuffer[Index] > Z))
        {
                if (Surface->DepthBuffer)
                        Surface->DepthBuffer[Index] = Z;
                const float Aleph = (float)Col.A / 255.0;
                const float InvAleph = 1.0 - Aleph;

                RGBA Background = *(RGBA *)&Surface->Buffer[Index], Blended;
                Blended.R = (uint8_t)(Col.R * Aleph + Background.R * InvAleph);
                Blended.G = (uint8_t)(Col.G * Aleph + Background.G * InvAleph);
                Blended.B = (uint8_t)(Col.B * Aleph + Background.B * InvAleph);
                Blended.A = (uint8_t)(255);
                Surface->Buffer[Index] = *(uint32_t *)&Blended;
        }
}

void GDI2Pixel(SURFACE *Surface, float X, float Y, float Z, RGBA Col)
{
        switch (Surface->BPP)
        {
        case 32:
                GDI2Pixel32(Surface, X, Y, Z, Col);
                break;
        case 24:
                SerialPrint("'24 bits'\n");
                goto err;
        case 16:
                SerialPrint("'16 bits'\n");
                goto err;
        case 8:
                SerialPrint("'8 bits'\n");
                goto err;
        default:
        err:
                SerialPrint(", We only support 32 bit colour\n");
                break;
        }
}

void GDI2DrawRectDisplay(DISPLAY *Display, RECT *Rect)
{
        if (!Display || !Rect)
                return;

        float minX = Rect->Points[0].X;
        float maxX = Rect->Points[0].X;
        float minY = Rect->Points[0].Y;
        float maxY = Rect->Points[0].Y;

        for (int i = 1; i < 4; i++)
        {
                if (Rect->Points[i].X < minX)
                        minX = Rect->Points[i].X;
                if (Rect->Points[i].X > maxX)
                        maxX = Rect->Points[i].X;
                if (Rect->Points[i].Y < minY)
                        minY = Rect->Points[i].Y;
                if (Rect->Points[i].Y > maxY)
                        maxY = Rect->Points[i].Y;
        }

        float width = maxX - minX;
        float height = maxY - minY;
        SURFACE Surface = {
                .W = Display->W,
                .H = Display->H,
                .BPP = Display->BPP,
                .Buffer = Display->Framebuffer,
                .DepthBuffer = Display->DepthBuffer,
                .FOV = Display->FOV,
                .Z = 1.0,
                .X = 0,
                .Y = 0,
        };

        for (float y = minY; y <= maxY; y += 1.0)
        {
                float v = height ? (float)(y - minY) / height : 0;
                for (float x = minX; x <= maxX; x += 1.0)
                {
                        float u = width ? (float)(x - minX) / width : 0;

                        RGBA top;
                        top.R = (uint8_t)(Rect->Points[0].Col.R * (1 - u) + Rect->Points[1].Col.R * u);
                        top.G = (uint8_t)(Rect->Points[0].Col.G * (1 - u) + Rect->Points[1].Col.G * u);
                        top.B = (uint8_t)(Rect->Points[0].Col.B * (1 - u) + Rect->Points[1].Col.B * u);
                        top.A = (uint8_t)(Rect->Points[0].Col.A * (1 - u) + Rect->Points[1].Col.A * u);

                        RGBA bottom;
                        bottom.R = (uint8_t)(Rect->Points[3].Col.R * (1 - u) + Rect->Points[2].Col.R * u);
                        bottom.G = (uint8_t)(Rect->Points[3].Col.G * (1 - u) + Rect->Points[2].Col.G * u);
                        bottom.B = (uint8_t)(Rect->Points[3].Col.B * (1 - u) + Rect->Points[2].Col.B * u);
                        bottom.A = (uint8_t)(Rect->Points[3].Col.A * (1 - u) + Rect->Points[2].Col.A * u);

                        RGBA final;
                        final.R = (uint8_t)(top.R * (1 - v) + bottom.R * v);
                        final.G = (uint8_t)(top.G * (1 - v) + bottom.G * v);
                        final.B = (uint8_t)(top.B * (1 - v) + bottom.B * v);
                        final.A = (uint8_t)(top.A * (1 - v) + bottom.A * v);

                        float z = (float)(Rect->Points[0].Z * (1 - u) * (1 - v) +
                                          Rect->Points[1].Z * u * (1 - v) +
                                          Rect->Points[2].Z * u * v +
                                          Rect->Points[3].Z * (1 - u) * v);
                        GDI2Pixel(&Surface, x, y, z, final);
                }
        }
}

void GDI2DrawRect(SURFACE *Surface, RECT *Rect)
{
        if (!Surface || !Rect)
                return;

        float minX = Rect->Points[0].X;
        float maxX = Rect->Points[0].X;
        float minY = Rect->Points[0].Y;
        float maxY = Rect->Points[0].Y;

        for (int i = 1; i < 4; i++)
        {
                if (Rect->Points[i].X < minX)
                        minX = Rect->Points[i].X;
                if (Rect->Points[i].X > maxX)
                        maxX = Rect->Points[i].X;
                if (Rect->Points[i].Y < minY)
                        minY = Rect->Points[i].Y;
                if (Rect->Points[i].Y > maxY)
                        maxY = Rect->Points[i].Y;
        }

        float width = maxX - minX;
        float height = maxY - minY;

        for (float y = minY; y <= maxY; y += 1.0)
        {
                float v = height ? (float)(y - minY) / height : 0;
                for (float x = minX; x <= maxX; x += 1.0)
                {
                        float u = width ? (float)(x - minX) / width : 0;

                        RGBA top;
                        top.R = (uint8_t)(Rect->Points[0].Col.R * (1 - u) + Rect->Points[1].Col.R * u);
                        top.G = (uint8_t)(Rect->Points[0].Col.G * (1 - u) + Rect->Points[1].Col.G * u);
                        top.B = (uint8_t)(Rect->Points[0].Col.B * (1 - u) + Rect->Points[1].Col.B * u);
                        top.A = (uint8_t)(Rect->Points[0].Col.A * (1 - u) + Rect->Points[1].Col.A * u);

                        RGBA bottom;
                        bottom.R = (uint8_t)(Rect->Points[3].Col.R * (1 - u) + Rect->Points[2].Col.R * u);
                        bottom.G = (uint8_t)(Rect->Points[3].Col.G * (1 - u) + Rect->Points[2].Col.G * u);
                        bottom.B = (uint8_t)(Rect->Points[3].Col.B * (1 - u) + Rect->Points[2].Col.B * u);
                        bottom.A = (uint8_t)(Rect->Points[3].Col.A * (1 - u) + Rect->Points[2].Col.A * u);

                        RGBA final;
                        final.R = (uint8_t)(top.R * (1 - v) + bottom.R * v);
                        final.G = (uint8_t)(top.G * (1 - v) + bottom.G * v);
                        final.B = (uint8_t)(top.B * (1 - v) + bottom.B * v);
                        final.A = (uint8_t)(top.A * (1 - v) + bottom.A * v);

                        float z = (float)(Rect->Points[0].Z * (1 - u) * (1 - v) +
                                          Rect->Points[1].Z * u * (1 - v) +
                                          Rect->Points[2].Z * u * v +
                                          Rect->Points[3].Z * (1 - u) * v);
                        GDI2Pixel(Surface, x, y, z, final);
                }
        }
}

void GDI2BlitSurface(DISPLAY *Display, SURFACE *Surface)
{
        if (!Display || !Surface)
                return;
        if (Display->BPP != Surface->BPP)
                return;
        int64_t dstX = Surface->X;
        int64_t dstY = Surface->Y;
        int64_t srcX = 0;
        int64_t srcY = 0;
        int64_t copyW = Surface->W;
        int64_t copyH = Surface->H;
        if (dstX < 0) { srcX -= dstX; copyW += dstX; dstX = 0; }
        if (dstY < 0) { srcY -= dstY; copyH += dstY; dstY = 0; }
        if (dstX + copyW > Display->W) copyW = Display->W - dstX;
        if (dstY + copyH > Display->H) copyH = Display->H - dstY;
        if (copyW <= 0 || copyH <= 0)
                return;
        for (int64_t row = 0; row < copyH; ++row)
        {
                for (int64_t col = 0; col < copyW; ++col)
                {
                        const size_t SrcIdx = Surface->W * (srcY + row) + (srcX + col);
                        const size_t DstIdx = Display->W * (dstY + row) + (dstX + col);
                        if (Display->DepthBuffer)
                        {
                                float srcZ = Surface->DepthBuffer ? Surface->DepthBuffer[SrcIdx] : Surface->Z;
                                if (Display->DepthBuffer[DstIdx] <= srcZ)
                                        continue;
                                Display->DepthBuffer[DstIdx] = srcZ;
                        }
                        RGBA Src = *(RGBA *)&Surface->Buffer[SrcIdx];
                        RGBA Dst = *(RGBA *)&Display->Framebuffer[DstIdx];
                        const float A    = Src.A / 255.0f;
                        const float InvA = 1.0f - A;
                        RGBA Out;
                        Out.R = (uint8_t)(Src.R * A + Dst.R * InvA);
                        Out.G = (uint8_t)(Src.G * A + Dst.G * InvA);
                        Out.B = (uint8_t)(Src.B * A + Dst.B * InvA);
                        Out.A = 255;
                        Display->Framebuffer[DstIdx] = *(uint32_t *)&Out;
                }
        }
}

void GDI2BlitSurfaceToSurface(SURFACE *Dst, SURFACE *Src)
{
        if (!Dst || !Src)
                return;
        if (Dst->BPP != Src->BPP)
                return;

        int64_t dstX = Src->X;
        int64_t dstY = Src->Y;
        int64_t srcX = 0;
        int64_t srcY = 0;
        int64_t copyW = Src->W;
        int64_t copyH = Src->H;

        if (dstX < 0) { srcX -= dstX; copyW += dstX; dstX = 0; }
        if (dstY < 0) { srcY -= dstY; copyH += dstY; dstY = 0; }
        if (dstX + copyW > Dst->W) copyW = Dst->W - dstX;
        if (dstY + copyH > Dst->H) copyH = Dst->H - dstY;
        if (copyW <= 0 || copyH <= 0)
                return;

        for (int64_t row = 0; row < copyH; ++row)
        {
                for (int64_t col = 0; col < copyW; ++col)
                {
                        const size_t SrcIdx = Src->W * (srcY + row) + (srcX + col);
                        const size_t DstIdx = Dst->W * (dstY + row) + (dstX + col);
                        if (Dst->DepthBuffer)
                        {
                                float srcZ = Src->DepthBuffer ? Src->DepthBuffer[SrcIdx] : Src->Z;
                                if (Dst->DepthBuffer[DstIdx] <= srcZ)
                                        continue;
                                Dst->DepthBuffer[DstIdx] = srcZ;
                        }

                        RGBA S   = *(RGBA *)&Src->Buffer[SrcIdx];
                        RGBA D   = *(RGBA *)&Dst->Buffer[DstIdx];
                        const float A    = S.A / 255.0f;
                        const float InvA = 1.0f - A;
                        RGBA Out;
                        Out.R = (uint8_t)(S.R * A + D.R * InvA);
                        Out.G = (uint8_t)(S.G * A + D.G * InvA);
                        Out.B = (uint8_t)(S.B * A + D.B * InvA);
                        Out.A = 255;
                        Dst->Buffer[DstIdx] = *(uint32_t *)&Out;
                }
        }
}

void GDI2PartialCommit(DISPLAY *Display, SURFACE *Surface)
{
        if (!Display || !Surface)
                return;
        I32 startY = Surface->Y;
        I32 endY   = Surface->Y + Surface->H;
        I32 startX = Surface->X;
        I32 endX   = Surface->X + Surface->W;
        if (startY < 0)        startY = 0;
        if (endY > Display->H) endY   = Display->H;
        if (startX < 0)        startX = 0;
        if (endX > Display->W) endX   = Display->W;
        I32 copyW = endX - startX;
        if (copyW <= 0) return;
        for (I32 y = startY; y < endY; ++y)
        {
                const size_t Idx = y * Display->W + startX;
                memcpy(&Display->Front[Idx],
                       &Display->Framebuffer[Idx],
                       copyW * (Display->BPP >> 3));
        }
}

void GDI2Commit(DISPLAY *Display)
{
        memcpy(Display->Front, Display->Framebuffer,
               Display->W * Display->H *
               Display->BPP >> 3);
}

void GDI2DrawLine(SURFACE *Surface, POINT Points[2])
{
        if (!Surface || !Points)
                return;

        float x1 = Points[0].X;
        float y1 = Points[0].Y;
        float z1 = Points[0].Z;
        RGBA col1 = Points[0].Col;
        
        float x2 = Points[1].X;
        float y2 = Points[1].Y;
        float z2 = Points[1].Z;
        RGBA col2 = Points[1].Col;

        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        
        int sx = x1 < x2 ? 1 : -1;
        int sy = y1 < y2 ? 1 : -1;
        
        int err = dx - dy;
        
        float x = x1;
        float y = y1;
        
        float totalDist = sqrtf(dx*dx + dy*dy);
        if (totalDist < 0.001) totalDist = 1.0;        
        while (1)
        {
                float currentDist = sqrtf((x - x1)*(x - x1) + (y - y1)*(y - y1));
                float t = currentDist / totalDist;
                if (t > 1.0) t = 1.0;
                float z = z1 * (1 - t) + z2 * t;
                
                RGBA col;
                col.R = (uint8_t)(col1.R * (1 - t) + col2.R * t);
                col.G = (uint8_t)(col1.G * (1 - t) + col2.G * t);
                col.B = (uint8_t)(col1.B * (1 - t) + col2.B * t);
                col.A = (uint8_t)(col1.A * (1 - t) + col2.A * t);
                GDI2Pixel(Surface, x, y, z, col);
                
                if (x == x2 && y == y2)
                        break;
                
                int e2 = 2 * err;
                if (e2 > -dy)
                {
                        err -= dy;
                        x += sx;
                }
                if (e2 < dx)
                {
                        err += dx;
                        y += sy;
                }
        }
}

void GDI2DrawLineAA(SURFACE *Surface, POINT Points[2])
{
        if (!Surface || !Points)
                return;

        float x1 = Points[0].X;
        float y1 = Points[0].Y;
        float z1 = Points[0].Z;
        RGBA col1 = Points[0].Col;
        
        float x2 = Points[1].X;
        float y2 = Points[1].Y;
        float z2 = Points[1].Z;
        RGBA col2 = Points[1].Col;
        int steep = abs(y2 - y1) > abs(x2 - x1);
        
        if (steep)
        {
                float t;
                t = x1; x1 = y1; y1 = t;
                t = x2; x2 = y2; y2 = t;
        }
        
        if (x1 > x2)
        {
                float t;
                t = x1; x1 = x2; x2 = t;
                t = y1; y1 = y2; y2 = t;
                
                RGBA ct = col1;
                col1 = col2;
                col2 = ct;
                
                t = z1; z1 = z2; z2 = t;
        }
        
        float dx = x2 - x1;
        float dy = y2 - y1;
        float gradient = dx == 0 ? 1.0 : dy / dx;
        
        float xend = (float)(uint32_t)(x1);
        float yend = y1 + gradient * (xend - x1);
        
        float xpxl1 = xend;
        float ypxl1 = (float)(uint32_t)(yend);
        
        if (steep)
        {
                RGBA col;
                float z = z1;
                
                float brightness = 1 - (yend - ypxl1);
                col.R = (uint8_t)(col1.R * brightness);
                col.G = (uint8_t)(col1.G * brightness);
                col.B = (uint8_t)(col1.B * brightness);
                col.A = (uint8_t)(col1.A * brightness);
                
                GDI2Pixel(Surface, ypxl1, xpxl1, z, col);
                
                brightness = yend - ypxl1;
                col.R = (uint8_t)(col1.R * brightness);
                col.G = (uint8_t)(col1.G * brightness);
                col.B = (uint8_t)(col1.B * brightness);
                col.A = (uint8_t)(col1.A * brightness);
                
                GDI2Pixel(Surface, ypxl1 + 1, xpxl1, z, col);
        }
        else
        {
                RGBA col;
                float z = z1;
                
                float brightness = 1 - (yend - ypxl1);
                col.R = (uint8_t)(col1.R * brightness);
                col.G = (uint8_t)(col1.G * brightness);
                col.B = (uint8_t)(col1.B * brightness);
                col.A = (uint8_t)(col1.A * brightness);
                
                GDI2Pixel(Surface, xpxl1, ypxl1, z, col);
                
                brightness = yend - ypxl1;
                col.R = (uint8_t)(col1.R * brightness);
                col.G = (uint8_t)(col1.G * brightness);
                col.B = (uint8_t)(col1.B * brightness);
                col.A = (uint8_t)(col1.A * brightness);
                
                GDI2Pixel(Surface, xpxl1, ypxl1 + 1, z, col);
        }
        
        float intery = yend + gradient;
        
        xend = (float)(uint32_t)(x2);
        yend = y2 + gradient * (xend - x2);
        
        float xpxl2 = xend;
        float ypxl2 = (float)(uint32_t)(yend);
        
        if (steep)
        {
                RGBA col;
                float z = z2;
                
                float brightness = 1 - (yend - ypxl2);
                col.R = (uint8_t)(col2.R * brightness);
                col.G = (uint8_t)(col2.G * brightness);
                col.B = (uint8_t)(col2.B * brightness);
                col.A = (uint8_t)(col2.A * brightness);
                
                GDI2Pixel(Surface, ypxl2, xpxl2, z, col);
                
                brightness = yend - ypxl2;
                col.R = (uint8_t)(col2.R * brightness);
                col.G = (uint8_t)(col2.G * brightness);
                col.B = (uint8_t)(col2.B * brightness);
                col.A = (uint8_t)(col2.A * brightness);
                
                GDI2Pixel(Surface, ypxl2 + 1, xpxl2, z, col);
        }
        else
        {
                RGBA col;
                float z = z2;
                
                float brightness = 1 - (yend - ypxl2);
                col.R = (uint8_t)(col2.R * brightness);
                col.G = (uint8_t)(col2.G * brightness);
                col.B = (uint8_t)(col2.B * brightness);
                col.A = (uint8_t)(col2.A * brightness);
                
                GDI2Pixel(Surface, xpxl2, ypxl2, z, col);
                
                brightness = yend - ypxl2;
                col.R = (uint8_t)(col2.R * brightness);
                col.G = (uint8_t)(col2.G * brightness);
                col.B = (uint8_t)(col2.B * brightness);
                col.A = (uint8_t)(col2.A * brightness);
                
                GDI2Pixel(Surface, xpxl2, ypxl2 + 1, z, col);
        }
        
        for (float x = xpxl1 + 1; x < xpxl2; x++)
        {
                float t = (x - x1) / dx; // Interpolation factor
                if (t < 0) t = 0;
                if (t > 1) t = 1;
                
                float z = z1 * (1 - t) + z2 * t;
                RGBA col;
                col.R = (uint8_t)(col1.R * (1 - t) + col2.R * t);
                col.G = (uint8_t)(col1.G * (1 - t) + col2.G * t);
                col.B = (uint8_t)(col1.B * (1 - t) + col2.B * t);
                col.A = (uint8_t)(col1.A * (1 - t) + col2.A * t);
                
                if (steep)
                {
                        float brightness = 1 - (intery - (float)(uint32_t)(intery));
                        RGBA temp = col;
                        temp.R = (uint8_t)(col.R * brightness);
                        temp.G = (uint8_t)(col.G * brightness);
                        temp.B = (uint8_t)(col.B * brightness);
                        temp.A = (uint8_t)(col.A * brightness);
                        
                        GDI2Pixel(Surface, (float)(uint32_t)(intery), x, z, temp);
                        
                        brightness = intery - (float)(uint32_t)(intery);
                        temp.R = (uint8_t)(col.R * brightness);
                        temp.G = (uint8_t)(col.G * brightness);
                        temp.B = (uint8_t)(col.B * brightness);
                        temp.A = (uint8_t)(col.A * brightness);
                        
                        GDI2Pixel(Surface, (float)(uint32_t)(intery) + 1, x, z, temp);
                        
                        intery += gradient;
                }
                else
                {
                        float brightness = 1 - (intery - (float)(uint32_t)(intery));
                        RGBA temp = col;
                        temp.R = (uint8_t)(col.R * brightness);
                        temp.G = (uint8_t)(col.G * brightness);
                        temp.B = (uint8_t)(col.B * brightness);
                        temp.A = (uint8_t)(col.A * brightness);
                        
                        GDI2Pixel(Surface, x, (float)(uint32_t)(intery), z, temp);
                        
                        brightness = intery - (float)(uint32_t)(intery);
                        temp.R = (uint8_t)(col.R * brightness);
                        temp.G = (uint8_t)(col.G * brightness);
                        temp.B = (uint8_t)(col.B * brightness);
                        temp.A = (uint8_t)(col.A * brightness);
                        
                        GDI2Pixel(Surface, x, (float)(uint32_t)(intery) + 1, z, temp);
                        
                        intery += gradient;
                }
        }
}

void GDI2ClearSurface(SURFACE *Surface)
{
        if (!Surface)
                return;
        if (Surface->Buffer)
        {
                memset(Surface->Buffer, 0x00, Surface->W * Surface->H * (Surface->BPP >> 3));
        }
        if (Surface->DepthBuffer)
        {
                for (size_t i = 0; i < (size_t)Surface->W * (size_t)Surface->H; ++i)
                {
                        Surface->DepthBuffer[i] = INFINITY;
                }
        }
}

void GDI2ClearDisplay(DISPLAY *Display)
{
        if (!Display || !Display->Framebuffer)
                return;
        memset(Display->Framebuffer, 0, Display->W * Display->H * (Display->BPP >> 3));
        if (Display->DepthBuffer)
        {
                for (size_t i = 0; i < (size_t)Display->W * (size_t)Display->H; ++i)
                {
                        Display->DepthBuffer[i] = INFINITY;
                }
        }
}

