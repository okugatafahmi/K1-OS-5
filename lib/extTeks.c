#include "extTeks.h"
#include "video.h"

void readStringTeks(char *string, char *signal, int *len)
{
    int i = 0;
    char input = 0;
    int ax, bx, cx, dx;
    char posX, posY, minX;
    char isSignal = 0;

    while (isSignal == 0)
    {
        ax = 0;
        bx = 0;
        cx = 0;
        dx = 0;
        input = interruptEdit(0x16, &ax, &bx, &cx, &dx);

        if (input == '\b')
        {
            if (i > 0)
            {
                interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
                interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
                interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
                string[i--] = '\0';
            }
        }
        // Ctrl + O atau S atau X
        else if (input == CTRL_O || input == CTRL_S || input == CTRL_X)
        {
            isSignal = 1;
        }
        else if (ax == UP || ax == DOWN)
        {
        }
        else
        {
            if (input == '\r')
            {
                interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
                string[i] = '\n';
            }
            else
            {
                string[i] = input;
            }
            interrupt(0x10, 0xE00 + input, 0, 0, 0);
            i++;
        }
    }
    if (isSignal)
        *signal = input;
    else
        *signal = 0;
    string[i] = '\0';
    *len = i;
}