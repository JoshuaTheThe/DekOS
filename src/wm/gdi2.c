#include <wm/gdi2.h>
#include <drivers/dev/serial.h>
#include <string.h>
#include <drivers/math.h>

void GDI2Pixel32(SURFACE *Surface, float X, float Y, float Z, RGBA Col)
{
        if (!Surface || !Surface->Buffer || Z <= 0.1)
                return;
        int64_t SX = (size_t)(Surface->W / 2 + (X / Z) * Surface->FOV);
        int64_t SY = (size_t)(Surface->H / 2 - (Y / Z) * Surface->FOV);
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
        for (int64_t row = 0; row < Surface->H; ++row)
        {
                const size_t BufIDX = Surface->W * row;
                const size_t dstX = Surface->X;
                const size_t dstY = Surface->Y;

                if (dstX >= Display->W || dstY >= Display->H)
                        return;

                size_t copyW = Surface->W;
                if (dstX + copyW > Display->W)
                        copyW = Display->W - dstX;

                const size_t FrbIDX = Display->W * (dstY + row) + dstX;
                memcpy(&Display->Framebuffer[FrbIDX],
                       &Surface->Buffer[BufIDX],
                       copyW * (Surface->BPP / 8));
        }
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
