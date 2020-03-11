#define SECTOR_SIZE 512
#define MAX_BYTE 256
#define MAP_SECTOR 0x100
#define FILES_SECTOR 0X101
#define SECTORS_SECTOR 0x103
#define MAX_FILES 16
#define DIR_ENTRY_LENGTH 32
#define MAX_FILENAME 12
#define MAX_FILESECTOR 20
#define EMPTY 0x00
#define USED 0xFF
#define FILE_NOT_FOUND -1
#define FILE_HAS_EXIST -1
#define INSUFFICIENT_FILES -2
#define INSUFFICIENT_SECTORS -3
#define INVALID_FOLDER -4

/* Ini deklarasi fungsi */
void handleInterrupt21(int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
int div(int, int);
int mod(int, int);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void clear(char *buffer, int length); //Fungsi untuk mengisi buffer dengan 0
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void executeProgram(char *path, int segment, int *result, char parentIndex);

void printLogo();

int main()
{
  char buffer[SECTOR_SIZE * MAX_FILESECTOR];
  int success;
  printLogo();
  makeInterrupt21();
  interrupt(0x21, 0x4, buffer, "key.txt", &success);
  if (success)
  {
    interrupt(0x21, 0x0, "key.txt = ", 0, 0);
    interrupt(0x21, 0x0, buffer, 0, 0);
    interrupt(0x21, 0x0, "\n\r", 0, 0);
  }
  else
  {
    interrupt(0x21, 0x6, "milestone1", 0x2000, &success);
    if (success != 1)
    {
      interrupt(0x21, 0x0, "Failed to execute milestone1\n\r", 0, 0);
    }
  }
  while (1);
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
      interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
      interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
      interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
      if (i > 0)
      {
        i--;
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

int div(int a, int b)
{
  int count = 0;
  while (b < a)
  {
    a -= b;
    count++;
  }
  return count;
}

int mod(int a, int b)
{
  while (b < a)
  {
    a -= b;
  }
  return a;
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
  char dir[SECTOR_SIZE];
  int iterDir, iterFileName;
  int iterLastByte, iterSector;
  int found, equal; // untuk boolean file
  // Membaca sector dir untuk membaca semua file
  readSector(dir, DIR_SECTOR);

  // Iterasi setiap file
  found = 0;
  iterDir = 0;
  while (!found && iterDir < SECTOR_SIZE)
  {
    iterFileName = 0;
    equal = 1;
    // Mencari yang nama file-nya sama
    while (equal && filename[iterFileName] != '\0' && iterFileName < MAX_FILENAME)
    {
      if (dir[iterDir + iterFileName] != filename[iterFileName])
      {
        equal = 0;
      }
      else
      {
        ++iterFileName;
      }
    }
    if (equal && dir[iterDir + iterFileName] == '\0')
    {
      found = 1;
    }
    else
    {
      iterDir += DIR_ENTRY_LENGTH;
    }
  }

  if (found)
  {
    // menyimpan posisi awal sector file
    iterLastByte = iterDir + MAX_FILENAME;
    iterSector = 0;
    while (iterSector < MAX_FILESECTOR && dir[iterLastByte + iterSector] != 0)
    {
      readSector(buffer + iterSector * SECTOR_SIZE, dir[iterLastByte + iterSector]);
      ++iterSector;
    }
  }
  *success = found;
}

void clear(char *buffer, int length)
{
  int i;
  for (i = 0; i < length; i++)
  {
    buffer[i] = 0x00;
  }
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex)
{
  char dir[SECTOR_SIZE], map[SECTOR_SIZE], sectorBuffer[SECTOR_SIZE];
  int dirIndex;

  readSector(map, MAP_SECTOR);
  readSector(dir, DIR_SECTOR);
  // mencari dir kosong
  for (dirIndex = 0; dirIndex < MAX_FILES && dir[dirIndex * DIR_ENTRY_LENGTH] != '\0'; ++dirIndex)
  {
  }

  if (dirIndex < MAX_FILES)
  {
    int i, j, sector;
    // mengecek apakah banyak sector yang kosong mencukupi
    for (i = 0, sector = 0; i < MAX_BYTE && sector < *sectors; ++i)
    {
      if (map[i] == EMPTY)
      {
        ++sector;
      }
    }

    if (sector < *sectors)
    { // sector tidak cukup
      *sectors = INSUFFICIENT_SECTORS;
    }
    else
    { // sector cukup
      clear(dir + dirIndex * DIR_ENTRY_LENGTH, DIR_ENTRY_LENGTH);
      // mengisi nama
      for (i = 0; i < MAX_FILENAME && filename[i] != '\0'; ++i)
      {
        dir[dirIndex * DIR_ENTRY_LENGTH + i] = filename[i];
      }
      // mengisi sector
      i = 0; sector = 0;
      while (i < MAX_BYTE && sector < *sectors)
      {
        if (map[i] == EMPTY)
        {
          clear(sectorBuffer, SECTOR_SIZE);
          for (j = 0; j < SECTOR_SIZE; ++j)
          {
            sectorBuffer[j] = buffer[sector * SECTOR_SIZE + j];
          }
          writeSector(sectorBuffer, i);
          dir[dirIndex * DIR_ENTRY_LENGTH + MAX_FILENAME + sector] = i;
          map[i] = USED;
          ++sector;
        }
        ++i; 
      }
      writeSector(map, MAP_SECTOR);
      writeSector(dir, DIR_SECTOR);
    }
  }
  else
  {
    *sectors = INSUFFICIENT_DIR_ENTRIES;
  }
}

void executeProgram(char *filename, int segment, int *success, char parentIndex)
{
  int i;
  char buffer[SECTOR_SIZE * MAX_FILESECTOR];
  readFile(buffer, filename, success);
  if (*success)
  {
    for (i = 0; i < 20 * 512; i++)
    {
      putInMemory(segment, i, buffer[i]);
    }
    launchProgram(segment);
  }
}
