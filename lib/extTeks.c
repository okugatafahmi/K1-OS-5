#include "extTeks.h"
#include "video.h"
#include "defines.h"

void printStringTeksSub(char *string, int idxNow, char row, char col)
{
    while (row <= MAX_ROW && string[idxNow] == 0)
    {
        interrupt(0x10, (0xE * 256) + string[idxNow], 0, 0);
        ++col;
        if (col == MAX_COL + 1)
        {
            col == 0;
            ++row;
        }
        ++idxNow;
    }
}

void readStringTeks(char *string, char *signal, int *len)
{
    int i = 0, iLast = 0, offset;
    char input = 0, c;
    int ax, bx, cx, dx;
    char posRow, posCol, colMove, col;
    char isSignal = 0, isMoveVertical = 0;
    int lenPerRow[MAX_FILESECTOR * SECTOR_SIZE], idxRow = 0;

    while (isSignal == 0)
    {
        getPos(&posRow, &posCol);

        ax = 0;
        bx = 0;
        cx = 0;
        dx = 0;
        input = interruptEdit(0x16, &ax, &bx, &cx, &dx);

        if (input == '\b')
        {
            if (i > 0) // bukan karakter pertama
            {
                if (posCol == MIN_COL)
                {
                    if (posRow == MIN_ROW)
                    {
                        scrollDown();
                    }
                    --idxRow;
                    setPos(posRow - 1, lenPerRow[idxRow]);
                }
                else
                {
                    interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
                    interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
                    interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
                }
                string[i--] = '\0';
            }
        }
        // Ctrl + O atau S atau X
        else if (input == CTRL_O || input == CTRL_S || input == CTRL_X)
        {
            isSignal = 1;
        }
        else if (ax == UP || ax == DOWN || ax == LEFT || ax == RIGHT)
        {
            if (ax == UP && idxRow > 0)
            {
                if (posRow == MIN_ROW)
                {
                    scrollDown();
                    getPos(&posRow, &posCol);
                    setPos(MIN_ROW, MIN_COL);
                    c = string[i - posCol];
                    string[i - posCol] = '\0';
                    offset = findIdxBefore(string, LINE_SCROLL, i - posCol - 1);
                    interrupt(0x21, 0, string + offset, 0, 0);
                    string[i - posCol] = c;
                    setPos(posRow, posCol);
                }
                if (isMoveVertical)
                {
                    col = ((lenPerRow[idxRow - 1] - 1 >= colMove) ? colMove : lenPerRow[idxRow - 1] - 1);
                }
                else if (lenPerRow[idxRow - 1] - 1 < posCol)
                {
                    col = lenPerRow[idxRow - 1] - 1;
                }
                else
                {
                    col = posCol;
                }
                --idxRow;
                i = sigmaLenPerRow(lenPerRow, idxRow) + (int)col;
                setPos(posRow - 1, col);
            }
            else if (ax == DOWN && lenPerRow[idxRow + 1] != 0)
            {
                if (posRow == MAX_ROW)
                {
                    scrollUp();
                    getPos(&posRow, &posCol);
                    i = sigmaLenPerRow(lenPerRow, idxRow + 1);
                    printStringTeksSub(string, i, posRow, posCol);
                    setPos(posRow, posCol);
                    i += posCol;
                }
                if (isMoveVertical)
                {
                    col = ((lenPerRow[idxRow + 1] - 1 >= colMove) ? colMove : lenPerRow[idxRow + 1] - 1);
                }
                else if (lenPerRow[idxRow + 1] - 1 < posCol)
                {
                    col = lenPerRow[idxRow + 1] - 1;
                }
                else
                {
                    col = posCol;
                }
                ++idxRow;
                i = sigmaLenPerRow(lenPerRow, idxRow) + (int)col;
                setPos(posRow + 1, col);
            }
            else if (ax == RIGHT && i < iLast)
            {
                if (posCol == MAX_COL)
                {
                    setPos(posRow + 1, MIN_COL);
                    ++idxRow;
                }
                else
                {
                    setPos(posRow, posCol + 1);
                }
                ++i;
            }
            else if (ax == LEFT && i > 0)
            {
                if (posCol == MIN_COL)
                {
                    setPos(posRow - 1, lenPerRow[idxRow - 1] - 1);
                    --idxRow;
                }
                else
                {
                    setPos(posRow, posCol - 1);
                }
                --i;
            }
        }
        else
        {
            if (input == '\r')
            {
                if (posRow == MAX_ROW)
                    scrollUp();
                interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
                string[i] = '\n';
                lenPerRow[idxRow++] = posCol + 1;
            }
            else
            {
                if (posRow == MAX_ROW && posCol == MAX_COL)
                {
                    scrollUp();
                }
                string[i] = input;
            }
            interrupt(0x10, 0xE00 + input, 0, 0, 0);
            if (iLast == i)
            {
                ++iLast;
                lenPerRow[idxRow] = posCol + 2;
                if (posCol == MAX_COL)
                    ++idxRow;
            }
            ++i;
        }

        if (ax == UP || ax == DOWN)
        {
            if (!isMoveVertical)
            {
                isMoveVertical = TRUE;
                colMove = posCol;
            }
        }
        else if (isMoveVertical)
        {
            isMoveVertical = FALSE;
        }
    }
    if (isSignal)
        *signal = input;
    else
        *signal = 0;
    string[iLast] = '\0';
    *len = iLast;
}

int findIdxBefore(char *string, int nRow, int idxNow)
{
    int cntHuruf = 0;
    while (nRow > 0)
    {
        if (string[idxNow] == '\n')
        {
            --nRow;
        }
        else if (cntHuruf + 1 == MAX_COL - MIN_COL + 1)
        {
            cntHuruf = 0;
            --nRow;
        }
        else
        {
            ++cntHuruf;
        }
        ++idxNow;
    }
}

int sigmaLenPerRow(int *lenPerRow, int idxRow)
{
    int i, res = 0;
    for (i = 0; i < idxRow; ++i)
    {
        res += (lenPerRow[i] - 1);
    }
    return res;
}
