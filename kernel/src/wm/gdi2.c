#include <wm/gdi2.h>
#include <drivers/serial.h>
#include <string.h>

void GDI2Pixel32(SURFACE *Surface, float X, float Y, float Z, ColourRGBA Col)
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

                ColourRGBA Background = *(ColourRGBA *)&Surface->Buffer[Index], Blended;
                Blended.R = (uint8_t)(Col.R * Aleph + Background.R * InvAleph);
                Blended.G = (uint8_t)(Col.G * Aleph + Background.G * InvAleph);
                Blended.B = (uint8_t)(Col.B * Aleph + Background.B * InvAleph);
                Blended.A = (uint8_t)(255);
                Surface->Buffer[Index] = *(uint32_t *)&Blended;
        }
}

void GDI2Pixel(SURFACE *Surface, float X, float Y, float Z, ColourRGBA Col)
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

                        ColourRGBA top;
                        top.R = (uint8_t)(Rect->Points[0].Col.R * (1 - u) + Rect->Points[1].Col.R * u);
                        top.G = (uint8_t)(Rect->Points[0].Col.G * (1 - u) + Rect->Points[1].Col.G * u);
                        top.B = (uint8_t)(Rect->Points[0].Col.B * (1 - u) + Rect->Points[1].Col.B * u);
                        top.A = (uint8_t)(Rect->Points[0].Col.A * (1 - u) + Rect->Points[1].Col.A * u);

                        ColourRGBA bottom;
                        bottom.R = (uint8_t)(Rect->Points[3].Col.R * (1 - u) + Rect->Points[2].Col.R * u);
                        bottom.G = (uint8_t)(Rect->Points[3].Col.G * (1 - u) + Rect->Points[2].Col.G * u);
                        bottom.B = (uint8_t)(Rect->Points[3].Col.B * (1 - u) + Rect->Points[2].Col.B * u);
                        bottom.A = (uint8_t)(Rect->Points[3].Col.A * (1 - u) + Rect->Points[2].Col.A * u);

                        ColourRGBA final;
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
