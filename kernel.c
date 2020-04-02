#include "lib/defines.h"
#include "lib/math.h"
#include "lib/file.h"

/* Ini deklarasi fungsi */
void handleInterrupt21(int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void executeProgram(char *path, int segment, int *result, char parentIndex);
char compare2String(char* s1, char* s2);

void printLogo();

int main()
{
  char buffer[SECTOR_SIZE * MAX_FILESECTOR], command[SECTOR_SIZE];
  int success;
  printLogo();
  makeInterrupt21();
  while (1){
    printString("Anda mau apa: ");
    readString(command);

    executeProgram(command, 0x2000, &success, 0x01);
    if (success != 1)
    {
      interrupt(0x21, 0x0, "Failed to execute ", 0, 0);
      interrupt(0x21, 0x0, command, 0,0);
      interrupt(0x21, 0x0, "\n\r", 0, 0);
    }
  }
}

void handleInterrupt21(int AX, int BX, int CX, int DX)
{
  char AL, AH;
  AL = (char) (AX);
  AH = (char) (AX >> 8);
  switch (AL)
  {
  case 0x0:
    printString(BX);
    break;
  case 0x1:
    readString(BX);
    break;
  case 0x2:
    readSector(BX, CX);
    break;
  case 0x3:
    writeSector(BX, CX);
    break;
  case 0x4:
    readFile(BX, CX, DX, AH);
    break;
  case 0x5:
    writeFile(BX, CX, DX, AH);
    break;
  case 0x6:
    executeProgram(BX, CX, DX, AH);
    break;
  default:
    printString("Invalid interrupt");
    break;
  }
}

void printStringInMemory(char *string, int x, int y)
{
  int i = 0;
  while (string[i] != '\0' && string[i] != '\n')
  {
    int offset = 0x8000 + ((80 * y) + x) * 2;
    putInMemory(0xB000, offset, string[i]);
    putInMemory(0xB000, offset + 1, y + 1);
    i++;
    x++;
  }
}

void printLogo()
{
  printStringInMemory("* * * * * * * * * * * * * * * * * * * * * * * * * * * *", 0, 0);
  printStringInMemory("* ______                                              *", 0, 1);
  printStringInMemory("* | ___ \\  ______  _____  _________     __      ___   *", 0, 2);
  printStringInMemory("* | |_/ / | ___   |     | ___   ___    /  \\    |   \\  *", 0, 3);
  printStringInMemory("* | ___ \\ | __    |     |    | |      /    \\   | | /  *", 0, 4);
  printStringInMemory("* | |_/ / | ___   |     |    | |     |  ||  |  |   \\  *", 0, 5);
  printStringInMemory("* \\____/  |_____  |     |    | |     |      |  |    \\ *", 0, 6);
  printStringInMemory("* * * * * * * * * * * * * * * * * * * * * * * * * * * *", 0, 7);
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

void readSector(char *buffer, int sector)
{
  interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1,
            mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector)
{
  interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1,
            mod(div(sector, 18), 2) * 0x100);
}

void executeProgram(char *filename, int segment, int *success, char parentIndex)
{
  int i;
  char buffer[SECTOR_SIZE * MAX_FILESECTOR];
  readFile(buffer, filename, success, parentIndex);
  if (*success == 1)
  {
    for (i = 0; i < SECTOR_SIZE * MAX_FILESECTOR; i++)
    {
      putInMemory(segment, i, buffer[i]);
    }
    launchProgram(segment);
  }
}
