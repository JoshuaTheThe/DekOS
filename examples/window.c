#include <wm/main.h>

int main()
{
        KRNLRES *Window =
            WMCreateWindow("Title",
                           0,
                           0,
                           128,
                           128);
        if (!Window)
        {
                return -1;
        }

        KRNLRES *Text =
            WMCreateElement(
                Window,
                0,
                0,
                100,
                32,
                WINDOW_ELEMENT_TEXT);
        
        if (!Text)
        {
                return -1;
        }
        
        WINDOW *Win = Window->Region.ptr;
        ELEMENT *Txt = Text->Region.ptr;
        /* Text is provided as a pointer array */
        char *Text[] = {"Hello, World!", NULL};
        Txt->ElementData.Text.Text = Text;
        Txt->ElementData.Text.Columns = 14;
        Txt->ElementData.Text.Lines = 1;
        Txt->ElementData.Text.Font = RenderGetFont();

        /* Never set, because of the WM Being unfinished */
        while (Win->State != WINDOW_CLOSED)
        {
                /* .. */
                char chr = getchar();

                if (WMGetFocused() != Window)
                {
                        /* Not our's, respect it
                         (this will be done by the
                         library eventually) */
                        continue;
                }

                /* Use ! to exit */
                if (chr == '!')
                {
                        break;
                }
        }

        ResourceReleaseK(Text);
        ResourceReleaseK(Window);
}