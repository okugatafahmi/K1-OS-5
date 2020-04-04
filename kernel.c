#include "lib/defines.h"
#include "lib/utils.h"
#include "lib/math.h"

/* Ini deklarasi fungsi */
void handleInterrupt21(int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void executeProgram(char *path, int segment, int *result, char parentIndex);

void printLogo();

int main()
{
  char buffer[SECTOR_SIZE * MAX_FILESECTOR], command[SECTOR_SIZE];
  int success;
  printLogo();
  makeInterrupt21();
  while (1)
  {
    printString("Anda mau apa: ");
    readString(command);

    executeProgram(command, 0x2000, &success, 0x01);
    if (success != 1)
    {
      interrupt(0x21, 0x0, "Failed to execute ", 0, 0);
      interrupt(0x21, 0x0, command, 0, 0);
      interrupt(0x21, 0x0, "\n\r", 0, 0);
    }
  }
}

void handleInterrupt21(int AX, int BX, int CX, int DX)
{
  char AL, AH;
  AL = (char)(AX);
  AH = (char)(AX >> 8);
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
    if (string[i] == '\n')
    {
      interrupt(0x10, (0xE * 256) + '\r', 0, 0);
    }
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
        string[i--] = '\0';
      }
    }
    else
    {
      string[i] = input;
      interrupt(0x10, 0xE00 + input, 0, 0, 0);
      if (input != '\r')
        i++;
    }
  }
  string[i] = '\0';
  interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
  interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
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

void readFile(char *buffer, char *path, int *result, char parentIndex)
{
  char files[SECTOR_FILES_SIZE];
  char sectors[SECTOR_SIZE];
  char pathTo[SECTOR_SIZE], filename[MAX_FILENAME];
  int iterSector, idxP = parentIndex, idxS;

  interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
  interrupt(0x21, 0x2, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);
  interrupt(0x21, 0x2, sectors, SECTORS_SECTOR, 0);

  splitPath(path, pathTo, filename);
  goToFolder(pathTo, result, &parentIndex);
  if (*result != 1)
  {
    *result = FILE_NOT_FOUND;
    return;
  }
  idxP = findIdxFilename(filename, parentIndex);
  if (idxP == FILE_NOT_FOUND)
  {
    *result = FILE_NOT_FOUND;
    return;
  }

  idxS = files[idxP * FILES_ENTRY_LENGTH + 1];
  if (idxS == 0xFF)
  { // jika folder
    *result = FILE_IS_DIRECTORY;
    return;
  }
  iterSector = 0;
  while (iterSector < MAX_FILESECTOR && sectors[idxS * MAX_FILESECTOR + iterSector] != 0)
  {
    interrupt(0x21, 0x2, buffer + iterSector * SECTOR_SIZE, sectors[idxS * MAX_FILESECTOR + iterSector], 0);
    ++iterSector;
  }
  *result = 1;
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex)
{
  char files[SECTOR_FILES_SIZE], map[SECTOR_SIZE], sectorBuffer[SECTOR_SIZE], sectorsFile[SECTOR_SIZE];
  char pathTo[SECTOR_SIZE], filename[MAX_FILENAME];
  int filesIndex, idxP, idxS, result;

  interrupt(0x21, 0x2, map, MAP_SECTOR, 0);
  interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
  interrupt(0x21, 0x2, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);
  interrupt(0x21, 0x2, sectorsFile, SECTORS_SECTOR, 0);

  splitPath(path, pathTo, filename);
  goToFolder(pathTo, &result, &parentIndex);
  if (result != 1)
  {
    *sectors = INVALID_FOLDER;
    return;
  }

  idxP = findIdxFilename(filename, parentIndex);
  if (idxP != FILE_NOT_FOUND)
  { // menemukan filename yang sama di parent index tsb
    *sectors = FILE_HAS_EXIST;
    return;
  }

  // mencari sektor files kosong
  for (filesIndex = 0; filesIndex < MAX_FILES && files[filesIndex * FILES_ENTRY_LENGTH + 2] != '\0'; ++filesIndex)
  {
  }

  if (filesIndex < MAX_FILES)
  {
    int i, j, emptySector;
    // cari sektor sektor_file yang kosong
    for (idxS = 0; idxS < MAX_SECTORS_FILESECTOR && sectorsFile[idxS * MAX_FILESECTOR] != '\0'; ++idxS)
    {
    }
    if (idxS == MAX_SECTORS_FILESECTOR)
    { // sector tidak cukup
      *sectors = INSUFFICIENT_SECTORS;
    }
    else
    { // sector cukup
      clear(files + filesIndex * FILES_ENTRY_LENGTH, FILES_ENTRY_LENGTH);
      clear(sectorsFile + idxS * MAX_FILESECTOR, MAX_FILESECTOR);
      files[filesIndex * FILES_ENTRY_LENGTH] = parentIndex;
      files[filesIndex * FILES_ENTRY_LENGTH + 1] = (char)idxS;
      // mengisi nama
      for (i = 0; i < MAX_FILENAME && path[i] != '\0'; ++i)
      {
        files[filesIndex * FILES_ENTRY_LENGTH + 2 + i] = filename[i];
      }
      // mengisi sector
      i = 0;
      emptySector = 0;
      while (emptySector < *sectors)
      {
        if (map[i] == EMPTY)
        {
          clear(sectorBuffer, SECTOR_SIZE);
          for (j = 0; j < SECTOR_SIZE; ++j)
          {
            sectorBuffer[j] = buffer[emptySector * SECTOR_SIZE + j];
          }
          interrupt(0x21, 0x3, sectorBuffer, i, 0);
          sectorsFile[idxS * MAX_FILESECTOR + emptySector++] = i;
          map[i] = USED;
        }
        ++i;
      }
      interrupt(0x21, 0x3, map, MAP_SECTOR, 0);
      interrupt(0x21, 0x3, files, FILES_SECTOR, 0);
      interrupt(0x21, 0x3, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);
      interrupt(0x21, 0x3, sectorsFile, SECTORS_SECTOR, 0);
      *sectors = TRUE;
    }
  }
  else
  {
    *sectors = INSUFFICIENT_FILES;
  }
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
