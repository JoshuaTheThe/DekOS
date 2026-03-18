
#include        <wm2/px.h>
#include        <memory/string.h>

void SwapEndian(void *ptr, size_t size){
        unsigned char *bytes = (unsigned char *)ptr;
        size_t i = 0;
        size_t j = size - 1;
        while (i < j){
                unsigned char tmp = bytes[i];
                bytes[i] = bytes[j];
                bytes[j] = tmp;
                i++;
                j--;
        }
}

U0      PXRender(SURFACE *Surface, PXImage *Image){
        if (!Image || !Surface){
                return;
        }

        GDI2ClearSurface(Surface);
        for (I32 y = 0; y < (I32)Image->Header.H; ++y)
        for (I32 x = 0; x < (I32)Image->Header.W; ++x)
        {
                if (!(ARGB_TO_BGRA(Image->Palette[Image->Image[y * Image->Header.W + x]]).R == 255 &&
                      ARGB_TO_BGRA(Image->Palette[Image->Image[y * Image->Header.W + x]]).G == 0x0 &&
                      ARGB_TO_BGRA(Image->Palette[Image->Image[y * Image->Header.W + x]]).B == 255))
                {
                        GDI2Pixel(Surface, x - (I32)Image->Header.W / 2, ((I32)Image->Header.H - y) - (I32)Image->Header.H / 2, 1.0,
                                ARGB_TO_BGRA(Image->Palette[Image->Image[y * Image->Header.W + x]]));
                }
        }
}

PXImage*PXLoad(SYSFILE *fp){
        PXImage  Image = {0};
        U32      Read = FRead((char *)&Image.Header, 1, sizeof(Image.Header), fp);
        if (Read < 10){
                printf(" [ERROR] Failed to read PX Image\n");
                return NULL;
        } else if (strncmp(Image.Header.PX,"PX",2)){
                printf(" [ERROR] File is not PX Image\n");
                return NULL;
        }

        Read = FRead((char *)Image.Palette, 1, sizeof(PXPalette), fp);
        if (Read < sizeof(PXPalette)){
                printf(" [ERROR] Failed to read PX Image's Palette\n");
                return NULL;
        }

        SwapEndian(&Image.Header.W, 4);
        SwapEndian(&Image.Header.H, 4);

        const U32 RequiredBytes = Image.Header.W * Image.Header.H;
        Image.Image = malloc(RequiredBytes);
        if (!Image.Image){
                printf(" [ERROR] Could not allocate %dx%d (%d) bytes for PX Image Data\n",
                        Image.Header.W,Image.Header.H,RequiredBytes);
        }

        Read = FRead((char *)Image.Image, 1, RequiredBytes, fp);
        if (Read < RequiredBytes){
                printf(" [ERROR] Failed to read from file, got %d, expected %d\n", Read, RequiredBytes);
        }

        PXImage *AllocatedImage = malloc(sizeof(Image));
        *AllocatedImage = Image;
        return AllocatedImage;
}
