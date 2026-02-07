#pragma pack(1)
#include <stdint.h>
#include <stdbool.h>
#include <utils.h>

#include <init/idt.h>
#include <init/gdt.h>

#include <drivers/features/feature.h>

#include <memory/alloc.h>
#include <memory/string.h>

#include <programs/scheduler.h>
#include <programs/elf/elf.h>

#include <drivers/dev/ps2/ps2.h>
#include <tty/output.h>
#include <tty/render/render.h>

#include <drivers/sys/pit.h>
#include <drivers/fs/iso9660.h>
#include <drivers/dev/sound/speaker.h>
#include <drivers/dev/sound/sb16.h>
#include <drivers/dev/serial.h>
#include <drivers/dev/parallel.h>
#include <drivers/dev/storage/ide.h>
#include <drivers/fs/storage.h>
#include <drivers/fs/fat.h>

#include <isr/system.h>

#include <drivers/dev/pci.h>
#include <resource/main.h>

#include <tty/render/render.h>
#include <tty/render/fonts.h>

#include <wm/main.h>

#include <forth.h>

#include <ini.h>

#include <tty/tty.h>

#include <init/pde.h>

typedef enum
{
        RESPONSE_WTF = 0,
        RESPONSE_HANDOVER_RESOURCE,
        RESPONSE_READ_FILE,
        RESPONSE_CREATE_PROC,
        RESPONSE_OK = 200,
} ResponseCode;

typedef struct __attribute__((__packed__))
{
        DWORD Code;

        union
        {
                void *P;
                BYTE bytes[200];
        } as;
} Response;

extern schedProcess_t processes[MAX_PROCS];
extern bool tty_needs_flushing;
extern RID rdFrameRID;
extern KRNLRES *fbRes;
extern char system_output[TTY_H][TTY_W];

uint8_t stdinstack[1024]  __attribute__((aligned(16)));

// WINDOW *KernelWindow = NULL;
// KRNLRES *KernelWindowResource = NULL;
volatile DWORD mx = 0, my = 0, pmx = 0, pmy = 0, mbuttons = 0;

void deleteTask(size_t i)
{
        if (processes[i].stack)
        {
                free(processes[i].stack);
        }

        if (processes[i].program)
        {
                free(processes[i].program);
        }

        memset(&processes[i], 0, sizeof(schedProcess_t));
}

/* Post-Init kernel stuff, e.g. managing procs, and communication */
// void kernelTask(multiboot_info_t *mbi)
// {
//         (void)mbi;
//         // speakerPlay(300);
//         // pitDelay(10);
//         // speakerStop();
//         // PROCID WMId = WMInit();
//         // RESULT Result = ResourceHandoverK(fbRes, WMId);
//         // font_t *Font = RenderGetFont();
//         // DWORD Width = Font->char_width * TTY_W;
//         // DWORD Height = Font->char_height * (TTY_H + 1);
//         // DWORD WWidth = Width + 16;
//         // DWORD WHeight = Height + 32;
//         // printf("Handover Result: %d, fbRes=%x\n", Result, fbRes->Region.ptr);
//         // KernelWindowResource = WMCreateWindow("Kernel Window", 10, 10, WWidth, WHeight);
//         // if (KernelWindowResource)
//         // {
//         //         KernelWindow = KernelWindowResource->Region.ptr;
//         //         char *X[TTY_H];
//         //         for (int i = 0; i < TTY_H; i++)
//         //         {
//         //                 X[i] = system_output[i];
//         //         }
//         //
//         //         DWORD CenterX = WMMiddlePointX(KernelWindow) - Width / 2;
//         //         DWORD CenterY = WMMiddlePointY(KernelWindow) - Height / 2;
//         //         KRNLRES *TextBuff = WMCreateElement(KernelWindowResource, CenterX, CenterY, Width, Height, WINDOW_ELEMENT_TEXT);
//         //         ((ELEMENT *)TextBuff->Region.ptr)->ElementData.Text.Font = Font;
//         //         ((ELEMENT *)TextBuff->Region.ptr)->ElementData.Text.Columns = TTY_W;
//         //         ((ELEMENT *)TextBuff->Region.ptr)->ElementData.Text.Lines = TTY_H;
//         //         ((ELEMENT *)TextBuff->Region.ptr)->ElementData.Text.Text = X;
//         //         (void)TextBuff;
//         // }
// }

Response KHandleRequest(size_t pidn, char *buf, size_t len, USERID User)
{
        (void)len;
        Response resp;
        Response *msg = (Response *)buf;
        switch (msg->Code)
        {
        case RESPONSE_OK:
                resp.Code = RESPONSE_OK;
                break;
        case RESPONSE_READ_FILE:
                resp.Code = RESPONSE_OK;
                resp.as.P = SMGetDrive()->ReadFile(SMGetDrive(), (char *)msg->as.bytes);
                break;
        case RESPONSE_CREATE_PROC:
        {
                bool iself;
                void *data = SMGetDrive()->ReadFile(SMGetDrive(), ((char**)&msg->as.bytes[4])[0]);
                size_t size = SMGetDrive()->FileSize(SMGetDrive(), ((char**)&msg->as.bytes[4])[0]);
                size_t pid = -1;

                if (data && size)
                        pid = elfLoadProgram(data, size, &iself, User, *(int*)msg->as.bytes, (char**)&msg->as.bytes[4]).num;
                if (data && size && iself)
                {
                        resp.Code = RESPONSE_OK;
                        *((uint32_t*)&resp.as.bytes[0]) = pid;
                }
                else
                {
                        resp.Code = RESPONSE_WTF;
                }
                break;
        }
        default:
                printf(" [CONFUSED] Incoming Message of unknown type from %d: %s\n", pidn, msg->as.bytes);
                strncpy((char *)resp.as.bytes, "wtf are you doing\n\0", 20);
                resp.Code = RESPONSE_WTF;
                break;
        }

        return resp;
}

/* Initialize the System */
void kmain(uint32_t magic, uint32_t mbinfo_ptr)
{
        multiboot_info_t *mbi;
        PDEInit();

        cli();
        while (magic != 0x2BADB002)
        {
                hlt();
        }

        mbi = (multiboot_info_t *)mbinfo_ptr;
        // setframebuffer(framebuffer);
        // setfont(&cascadia); /* bitmap */
        RenderSetFont(&cascadia);
        FeaturesInit();
        gdtInit();
        idtInit();
        memInit(mbi->mem_upper * 1024 + mbi->mem_lower * 1024);
        pitInit(250);
        schedInit();
        SerialInit();
        init_parallel_ports();
        SerialPrint("--DekOS--\r\nHello, World!\r\n");

        cli();
        fbRes = ResourceCreateK(NULL, RESOURCE_TYPE_RAW_FAR, 0, schedGetKernelPid(), NULL);
        fbRes->Region.ptr = (uint32_t *)mbi->framebuffer_addr;
        fbRes->Region.size = mbi->framebuffer_width * mbi->framebuffer_height * (mbi->framebuffer_bpp / 8);

        int nDim[3] = {mbi->framebuffer_width, mbi->framebuffer_height, 1};
        BOOL aDim[3] = {TRUE, TRUE, TRUE};
        RenderSetDim(nDim, aDim);

        if (!iso9660Init())
        {
                printf("Could not initialize ISO9660 Driver\n");
                sysHang();
        }

        if (!ps2_keyboard_present())
        {
                printf("Keyboard is not present, please plug one in\n");
                while (!ps2_keyboard_present())
                {
                        sti();
                        hlt();
                }
        }

        if (!ps2_mouse_present())
        {
                printf("Mouse is not present, please plug one in\n");
                while (!ps2_mouse_present())
                {
                        sti();
                        hlt();
                }
        }

        /* Stinky inconsistent naming for PS/2 stuff */
        ps2_initialize_mouse();
        ps2_initialize_keyboard();

        /* enumerate devices and save info to array */
        pciEnumerateDevices(pciRegister);
        SMInit();
        SMChange(4); /* Secondary ATA */
        FatTest(SMGetDrive());

//        for (int i = 0; i < kernel_symbols_count; ++i)
//        {
//                printf(" [INFO] Kernel Function '%s' present at %p\n", kernel_symbols[i].name, kernel_symbols[i].address);
//        }

        Ini Cfg = IniRead("system/system.ini");
        Ini *Saved = malloc(sizeof(*Saved));
        *Saved = Cfg;

        printf("%s\n", Saved->vars[0].value);
        const char *shell = IniGet(Saved, "shell_path");
        UsersLoad();
        USERID User = UserLogin();

        if (!shell)
        {
                printf(" [WARN] No Shell defined\n");
        }
        else
        {
                printf(" [INFO] Attemtping to launch shell @\"%s\"\n", shell);
                bool iself;
                void *data = SMGetDrive()->ReadFile(SMGetDrive(), shell);
                size_t size = SMGetDrive()->FileSize(SMGetDrive(), shell);

                if (data && size)
                        elfLoadProgram(data, size, &iself, User, 0, NULL);
                if (data && size && iself)
                {
                        printf(" [INFO] Shell started\n");
                }
                else
                {
                        printf(" [ERROR] The Shell @\"%s\" is invalid\n", shell);
                }
        }

        sti();

        while (true)
        {
                cli();
                for (int i = 0; i < MAX_PROCS; ++i)
                {
                        if (processes[i].delete)
                        {
                                cli();
                                deleteTask(i);
                                sti();
                        }
                }

                mouseFetch((int *)&mx, (int *)&my, (int *)&pmx, (int *)&pmy, (uint8_t *)&mbuttons);

                if (msgrecv(-1))
                {
                        char buf[4096];
                        size_t pidn = recvmsg(buf, 4096);
                        schedPid_t pid = {.num = pidn, .valid = progexists(pidn)};
                        Response resp = KHandleRequest(pidn, buf, 4096, schedGetProcessN(pid)->enactor);
                        sendmsg(pidn, &resp, sizeof(resp));
                }

                sti();

                display();
                hlt();
        }
}
