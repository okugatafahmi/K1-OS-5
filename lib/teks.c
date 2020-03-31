#include "teks.h"
#include "math.h"

void printInt(int num){
  char strNum[10005],isNegative=0;
  int i = 0, j;

  if (num==0){
    interrupt(0x10, (0xE * 256) + '0', 0, 0); 
  }
  else{
    if (num<0){
      num *= -1;
      isNegative = 1;
    }
    while(num>0){
      strNum[i++] = mod(num,10)+'0';
      num = div(num,10);
    }
    if (isNegative){
      interrupt(0x10, (0xE * 256) + '-', 0, 0);
    }
    for (j = i-1; j>=0; j--){
      interrupt(0x10, (0xE * 256) + strNum[j], 0, 0);
    }
  }
}

void printString(char *string)
{
  int i = 0;
  while (string[i] != '\0')
  {
    interrupt(0x10, (0xE * 256) + string[i], 0, 0);
    i++;
  }
}

void readString(char *string)
{
  int i = 0;
  char input = 0;
  while (input != '\r')
  {
    input = interrupt(0x16, 0, 0, 0, 0);

    if (input == '\b')
    {
      if (i > 0)
      {
        interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
        interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
        interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
        string[i--]='\0';
      }
    }
    else if (input == 24 || input==15)
    {
      break;
    }
    else
    {
      string[i] = input;
      interrupt(0x10, 0xE00 + input, 0, 0, 0);
      if (input != '\r')
        i++;
    }
  }
  if (input==24 || input==15) string[i]=input;
  else{
    string[i] = '\0';
    interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
    interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
  }
}