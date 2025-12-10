#include <wm/gdi.h>

extern KRNLRES *fbRes;

DWORD Colour;

void *(*ColourFunction)(DWORD,DWORD);
BOOL UseColourFunction=FALSE;
BOOL InColourFunction=FALSE;

RGBA ColourRGB(BYTE R, BYTE G, BYTE B)
{
        return (RGBA){.R = R, .G = G, .B = B, .A = 255};
}

DWORD ColourDword(RGBA Col)
{
        return *(DWORD *)&Col;
}

void SetColour(RGBA Col)
{
        if (!InColourFunction)
        {
                UseColourFunction = FALSE;
                ColourFunction = NULL;
        }
        Colour = ColourDword(Col);
}

RGBA GetColour(void)
{
        return *(RGBA *)&Colour;
}

void SetPixel(DWORD X, DWORD Y, RGBA Col)
{
        static DWORD Dim[3] = {-1};
        DWORD Colour = ColourDword(Col);
        if (fbRes->Owner.num != schedGetCurrentPid().num)
                return;
        if (Dim[0] == -1)
                RenderGetDim(Dim);
        if (X > Dim[0] || Y > Dim[1])
                return;
        ((DWORD *)fbRes->Region.ptr)[(Y)*Dim[0] + (X)] = Colour;
}

/**
 * GDIDrawRect - Draws a rectangle of given Dim (duh)
 */
void GDIDrawRect(DWORD X, DWORD Y, DWORD W, DWORD H)
{
        if (fbRes->Owner.num != schedGetCurrentPid().num)
                return;
        DWORD Dim[3];
        RenderGetDim(Dim);

        if (X > Dim[0] || Y > Dim[1])
                return;
        for (DWORD Yo = 0; Yo < H; ++Yo)
        {
                if (Yo + Y >= Dim[1])
                        break;
                for (DWORD Xo = 0; Xo < W; ++Xo)
                {
                        if (Xo + X >= Dim[0])
                                break;
                        if (UseColourFunction)
                        {
                                InColourFunction = TRUE;
                                ColourFunction(Xo, Yo);
                                InColourFunction = FALSE;
                        }
                        ((DWORD *)fbRes->Region.ptr)[(Y + Yo) * Dim[0] + (X + Xo)] = Colour;
                }
        }
}

/**
 * GDIDrawLine - Draws a Line of given Dim (duh)
 */
void GDIDrawLine(DWORD X1, DWORD Y1, DWORD X2, DWORD Y2)
{
        if (fbRes->Owner.num != schedGetCurrentPid().num)
                return;

        DWORD Dim[3];
        RenderGetDim(Dim);

        // Clamp coordinates to framebuffer bounds
        if (X1 >= Dim[0])
                X1 = Dim[0] - 1;
        if (Y1 >= Dim[1])
                Y1 = Dim[1] - 1;
        if (X2 >= Dim[0])
                X2 = Dim[0] - 1;
        if (Y2 >= Dim[1])
                Y2 = Dim[1] - 1;

        DWORD dx = (X2 > X1) ? (X2 - X1) : (X1 - X2);
        DWORD dy = (Y2 > Y1) ? (Y2 - Y1) : (Y1 - Y2);

        DWORD sx = (X1 < X2) ? 1 : -1;
        DWORD sy = (Y1 < Y2) ? 1 : -1;

        DWORD err = (dx > dy ? dx : -dy) / 2;
        DWORD e2;

        DWORD *fb = (DWORD *)fbRes->Region.ptr;
        DWORD stride = Dim[0];

        while (1)
        {
                // Draw current pixel
                if (UseColourFunction)
                {
                        InColourFunction = TRUE;
                        ColourFunction(X1, Y1);
                        InColourFunction = FALSE;
                }
                fb[Y1 * stride + X1] = Colour;

                if (X1 == X2 && Y1 == Y2)
                        break;

                e2 = err;
                if (e2 > -dx)
                {
                        err -= dy;
                        X1 += sx;
                }
                if (e2 < dy)
                {
                        err += dx;
                        Y1 += sy;
                }
        }
}

/**
 * Use a Function as the colour
 */
void SetColourFn(void *(*Foo)(DWORD,DWORD))
{
        ColourFunction = Foo;
        UseColourFunction = TRUE;
        InColourFunction = FALSE;
}

/**
 * GDISetDither - Set the Colour to Dither between Two colours for a given level
 */
void GDISetDither(DWORD X, DWORD Y, RGBA A, RGBA B, BYTE Level)
{
        DWORD pattern = (X ^ Y) * 16777619;
        if (!InColourFunction)
        {
                UseColourFunction = FALSE;
                ColourFunction = NULL;
        }

        if ((pattern & 0xFF) < Level)
        {
                Colour = ColourDword(A);
        }
        else
        {
                Colour = ColourDword(B);
        }
}
