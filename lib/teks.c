#include "teks.h"
#include "math.h"

void printInt(int num)
{
  char strNum[10005], isNegative = 0;
  int i = 0, j;

  if (num == 0)
  {
    interrupt(0x10, (0xE * 256) + '0', 0, 0, 0);
  }
  else
  {
    if (num < 0)
    {
      num *= -1;
      isNegative = 1;
    }
    while (num > 0)
    {
      strNum[i++] = mod(num, 10) + '0';
      num = div(num, 10);
    }
    if (isNegative)
    {
      interrupt(0x10, (0xE * 256) + '-', 0, 0, 0);
    }
    for (j = i - 1; j >= 0; j--)
    {
      interrupt(0x10, (0xE * 256) + strNum[j], 0, 0, 0);
    }
  }
}

void printString(char *string)
{
  interrupt(0x21, 0x0, string, 0, 0);
}

void readString(char *string)
{
  interrupt(0x21, 0x1, string, 0, 0);
}