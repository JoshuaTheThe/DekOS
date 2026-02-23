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
#include <drivers/fs/file.h>

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

void sysHang(void);

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

multiboot_info_t mbi;

/* Initialize the System */
void kmain(uint32_t magic, uint32_t mbinfo_ptr)
{
        mbi = *(multiboot_info_t *)mbinfo_ptr;
        PDEInit();

        cli();
        while (magic != 0x2BADB002)
        {
                hlt();
        }

        // setframebuffer(framebuffer);
        // setfont(&systemfont); /* bitmap */
        systemfont = &font_8x8;
        RenderSetFont(&font_8x8);
        FeaturesInit();
        gdtInit();
        idtInit();
        pitInit(250);
        memInit(mbi.mem_upper * 1024 + mbi.mem_lower * 1024);
        schedInit();
        SerialInit();
        init_parallel_ports();

        fbRes = ResourceCreateK(NULL, RESOURCE_TYPE_BITMAP_IMAGE, 0, schedGetKernelPid(), NULL);
        printf(" [DEBUG] sizeof(KRNLRES)=%d, offsetof(rid)=%d\n",
               sizeof(KRNLRES), offsetof(KRNLRES, rid));

        if (!fbRes)
        {
                printf(" [ERROR] Could not create framebuffer resource\n");
                sysHang();
        }
        fbRes->Region.ptr = (uint32_t *)mbi.framebuffer_addr;
        fbRes->Region.size = mbi.framebuffer_width * mbi.framebuffer_height * (mbi.framebuffer_bpp / 8);
        fbRes->OnHeap = FALSE;

        printf(" [INFO] created frame buffer of Rid %d\n", fbRes->rid);
        int nDim[3] = {mbi.framebuffer_width, mbi.framebuffer_height, 1};
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

        SMGetDrive()->WriteFile(SMGetDrive(), "balls.txt", "balls", 5);

        /** Brought back the ancient EnneaOS fetch cause its cool */
        printf("\n        /\\_/\\\n");
        printf("       | o o |\n");
        printf("/-----/-------\\------\\  EnneaOS uh i mean DekOS\n");
        printf("|    This Is The,    |  Version %s\n", __VER__);
        printf("| The The The The :3 |  %dMiB of Ram\n", (mbi.mem_upper * 1024 + mbi.mem_lower * 1024) / (1024 * 1024));
        printf("|              -josh |  %dMiB of Heap\n", (_heap_end-_heap_start) / (1024*1024));
        printf("\\--------------------/\n\n");

        Ini Cfg = IniRead("system/system.ini");
        Ini *Saved = malloc(sizeof(*Saved));
        *Saved = Cfg;
        const char *shell = IniGet(Saved, "shell_path");

        {
                const char *_tty_bg = IniGet(Saved, "tty_bg");
                const char *_tty_fg = IniGet(Saved, "tty_fg");
                const char *font = IniGet(Saved, "font");
                extern uint32_t tty_bg;
                extern uint32_t tty_fg;
                if (_tty_bg)
                        tty_bg = strthex(_tty_bg);
                if (_tty_fg)
                        tty_fg = strthex(_tty_fg);
                if (font)
                {
                        systemfont = FontLoad(font);
                        RenderSetFont(systemfont);
                }
        }
        UsersLoad();
        USERID User = UserLogin();

        schedPid_t pid = {0};

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
                        pid = elfLoadProgram(data, size, &iself, User, 0, (char **)&shell);
                if (data && size && iself)
                {
                        printf(" [INFO] Shell started\n");
                }
                else
                {
                        printf(" [ERROR] The Shell @\"%s\" is invalid, press enter to reboot\n", shell);
                        while (ps2_getchar() != '\n')
                                ;
                }
        }

        PROCID Wm = WMInit();
        KRNLRES *Window;
        if (Wm.num == 0 || !Wm.valid)
        {
                printf(" [ERROR] Failed to start window manager\n");\
        }
        else
        {
                const size_t Padding = IniGet(&Cfg, "wm_padding") ? atoi(IniGet(&Cfg, "wm_padding"), NULL) : WINDOW_PADDING_DEFAULT;
                const size_t Thickness = IniGet(&Cfg, "wm_thickness") ? atoi(IniGet(&Cfg, "wm_thickness"), NULL) : WINDOW_THICKNESS_DEFAULT;
                const RGBA Outer = IniGet(&Cfg, "wm_outer") ? ColourRGBD(strthex(IniGet(&Cfg, "wm_outer"))) : WINDOW_OUTER_DEFAULT;
                const RGBA Inner = IniGet(&Cfg, "wm_inner") ? ColourRGBD(strthex(IniGet(&Cfg, "wm_inner"))) : WINDOW_INNER_DEFAULT;
                const RGBA Border = IniGet(&Cfg, "wm_border") ? ColourRGBD(strthex(IniGet(&Cfg, "wm_border"))) : WINDOW_BORDER_DEFAULT;
                const DWORD TitleBarHeight = IniGet(&Cfg, "wm_titlebar_height") ? atoi(IniGet(&Cfg, "wm_titlebar_height"), NULL) : WINDOW_TITLEBAR_HEIGHT_DEFAULT;
                Window = WMCreateWindow("system",
                                        WINDOW_POSITION_DEFAULT_X,
                                        WINDOW_POSITION_DEFAULT_Y,
                                        TTY_W * systemfont->char_width,
                                        Thickness * 2 + Padding * 2 + TitleBarHeight * 2 + (TTY_H + 1) * systemfont->char_height,
                                        Padding,
                                        Thickness,
                                        TitleBarHeight,
                                        Outer,
                                        Inner,
                                        Border);
                KRNLRES *Element = WMCreateElement(Window, 0, 0, TTY_W * systemfont->char_width, (TTY_H + 1) * systemfont->char_height, WINDOW_ELEMENT_TEXT);
                ((ELEMENT *)Element->Region.ptr)->ElementData.Text.Font = systemfont;
                ((ELEMENT *)Element->Region.ptr)->ElementData.Text.Columns = TTY_W;
                ((ELEMENT *)Element->Region.ptr)->ElementData.Text.Lines = TTY_H;

                extern uint32_t tty_bg;
                extern uint32_t tty_fg;

                ((ELEMENT *)Element->Region.ptr)->ElementData.Text.Fg = ColourRGBD(tty_fg);
                ((ELEMENT *)Element->Region.ptr)->ElementData.Text.Bg = ColourRGBD(tty_bg);

                char **p = malloc(TTY_H * sizeof(char *));
                for (size_t i = 0; i < TTY_H; ++i)
                {
                        p[i] = system_output[i];
                }
                ((ELEMENT *)Element->Region.ptr)->ElementData.Text.Text = p;
                ResourceHandoverK(fbRes, Wm);
        }
        sti();

        while (true)
        {
                static uint64_t last_cleanup = 0;
                const uint64_t now = tick_counter;
                const uint64_t cleanup_interval = 1000;
                if (now - last_cleanup > cleanup_interval)
                {
                        if (!schedValidatePid(pid))
                        {
                                cli();
                                break;
                        }

                        for (int i = 0; i < MAX_PROCS; ++i)
                        {
                                if (processes[i].delete)
                                {
                                        cli();
                                        deleteTask(i);
                                        sti();
                                }
                        }

                        last_cleanup = now;
                }

                cli();
                mouseFetch((int *)&mx, (int *)&my, (int *)&pmx, (int *)&pmy, (uint8_t *)&mbuttons);
                sti();
                if (fbRes->Owner.num == 0)
                {
                        ResourceHandoverK(fbRes, Wm);
                }
                ((WINDOW *)(Window->Region.ptr))->RequiresRedraw = redraw;
                hlt();
        }

        cli();
        outw(0xB004, 0x2000);
        outw(0x604, 0x2000);
        outw(0x4004, 0x3400);

        outb(0x20, 0x11);
        outb(0xA0, 0x11);
        outb(0x21, 0x20);
        outb(0xA1, 0x28);
        outb(0x21, 0x04);
        outb(0xA1, 0x02);
        outb(0x21, 0x01);
        outb(0xA1, 0x01);
        outb(0x21, 0x0);
        outb(0xA1, 0x0);

        outb(0x64, 0xFE);

        cli();
        hlt();
}
