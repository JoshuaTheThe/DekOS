
#include        <wm2/px.h>
#include        <memory/string.h>

PXImage PXLoad(SYSFILE *fp){
        PXImage  Image = {0};
        U32      Read = FRead((char *)&Image.Header, sizeof(Image.Header), 1, fp);
        if (Read < 10){
                printf(" [ERROR] Failed to read PX Image\n");
                return Image;
        } else if (strncmp(Image.Header.PX,"PX",2)){
                printf(" [ERROR] File is not PX Image\n");
                return Image;
        }

        Read = FRead((char *)Image.Palette, sizeof(PXPalette), 1, fp);
        if (Read < sizeof(PXPalette)){
                printf(" [ERROR] Failed to read PX Image's Palette\n");
                return Image;
        }

        const U32 RequiredBytes = Image.Header.W * Image.Header.H;
        Image.Image = malloc(RequiredBytes);
        if (!Image.Image){
                printf(" [ERROR] Could not allocate %dx%d (%d) bytes for PX Image Data\n",
                        Image.Header.W,Image.Header.H,RequiredBytes);
        }

        FRead((char *)Image.Image, RequiredBytes, 1, fp);
        return Image;
}
