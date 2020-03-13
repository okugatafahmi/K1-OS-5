#define SECTOR_SIZE 512
#define MAP_SECTOR 0x100
#define FILES_SECTOR 0X101
#define SECTORS_SECTOR 0x103
#define MAX_FILES 64
#define MAX_SECTORS_FILESECTOR 32
#define FILES_ENTRY_LENGTH 16
#define MAX_FILENAME 14
#define MAX_FILESECTOR 16
#define EMPTY 0x00
#define USED 0xFF
#define FILE_NOT_FOUND -1
#define FILE_HAS_EXIST -1
#define INSUFFICIENT_FILES -2
#define INSUFFICIENT_SECTORS -3
#define INVALID_FOLDER -4
#define TRUE 1
#define FALSE 0

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
char compare2String(char* s1, char* s2);

void printLogo();

int main()
{
  char buffer[SECTOR_SIZE * MAX_FILESECTOR], command[SECTOR_SIZE];
  int success;
  printLogo();
  makeInterrupt21();
  while (1){
    // printString("Anda mau apa: ");
    // readString(command);

    executeProgram("shell", 0x2000, &success, 0xFF);
    if (success != 1)
    {
      interrupt(0x21, 0x0, "Failed to execute milestone1\n\r", 0, 0);
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
        i--;
      }
    }
    else if (input == 14)
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
  if (input==14) string[i]=14;
  else{
    string[i] = '\0';
    interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
    interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
  }
}

int div(int a, int b)
{
  int count = 0;
  while (b <= a)
  {
    a -= b;
    count++;
  }
  return count;
}

int mod(int a, int b)
{
  while (b <= a)
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


char compare2String(char* s1, char* s2){
  int i;
  for (i = 0; i<MAX_FILENAME; ++i){
    if (s1[i]=='\0' && s2[i]=='\0') return TRUE;
    if (s1[i]!=s2[i]) return FALSE;
  }
}

int searchRecurr(char *files, char *path, char parentIndex, char searchFolder, char *filename){
  char depan[MAX_FILENAME];
  char sisa[MAX_FILES*MAX_FILENAME];
  int i = 0,j;
  char isGoing = TRUE;
  char isFolder = FALSE;
  char isFound = FALSE;
  int idxP;
  while(i<MAX_FILENAME && isGoing){
    if (path[i]=='\0'){
      isGoing = FALSE;
    }
    else if (path[i]=='/'){
      isGoing = FALSE;
      isFolder = TRUE; 
    }
    else{
      depan[i] = path[i];
      i++;
    }
  }
  depan[i] = '\0';

  // cari yang sama
  idxP = 0;
  while(idxP<MAX_FILES && !isFound){
      if (files[idxP*FILES_ENTRY_LENGTH]==parentIndex && compare2String(files+idxP*FILES_ENTRY_LENGTH+2,depan)){
        isFound = TRUE;
      }
      else ++idxP;
  }
  
  // kalau ga ketemu
  if (!isFound){
    // untuk write file
    if (searchFolder && path[i]=='\0') {
      for (i=0; depan[i]!='\0'; ++i){
        filename[i] = depan[i];
      }
      filename[i] = '\0';
      return parentIndex;
    }
    else {
      return FILE_NOT_FOUND;
    }
  }
  else if (isFolder){ // kalau folder
    // ambil bagian belakang  
    for (j = i+1; j<MAX_FILENAME && path[j]!='\0'; ++j){
      sisa[j-(i+1)]= path[j];
    }
    sisa[j-(i+1)] = '\0';
    // pass ke recurr baru
    return searchRecurr(files, sisa, idxP, searchFolder, filename);
  }
  else{
    return idxP;
  }
}

void readFile(char *buffer, char *path, int *result, char parentIndex)
{
  char files[2*SECTOR_SIZE];
  char sectors[SECTOR_SIZE];
  char filename[MAX_FILENAME];
  int iterSector, idxP, idxS;

  readSector(files, FILES_SECTOR);
  readSector(files+SECTOR_SIZE, FILES_SECTOR+1);
  readSector(sectors, SECTORS_SECTOR);
  
  idxP = searchRecurr(files, path, parentIndex, 0, filename);
  if (idxP == FILE_NOT_FOUND){
    *result = FILE_NOT_FOUND;
    return;
  }

  idxS = files[idxP * FILES_ENTRY_LENGTH+1];
  iterSector = 0;
  while (iterSector < MAX_FILESECTOR && sectors[idxS * MAX_FILESECTOR + iterSector] != 0)
  {
    readSector(buffer + iterSector * SECTOR_SIZE, sectors[idxS * MAX_FILESECTOR + iterSector]);
    ++iterSector;
  }
  *result = 1;
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
  char files[2*SECTOR_SIZE], map[SECTOR_SIZE], sectorBuffer[SECTOR_SIZE], sectorsFile[SECTOR_SIZE], filename[MAX_FILENAME];
  int filesIndex, idxP, idxS;

  readSector(map, MAP_SECTOR);
  readSector(files, FILES_SECTOR);
  readSector(files+SECTOR_SIZE, FILES_SECTOR+1);
  readSector(sectorsFile, SECTORS_SECTOR);
  idxP = searchRecurr(files, path, parentIndex, 1, filename);

  if (idxP == FILE_NOT_FOUND){
    *sectors = INVALID_FOLDER;
    return;
  }
  // mencari sektor files kosong
  for (filesIndex = 0; filesIndex < MAX_FILES && files[filesIndex * FILES_ENTRY_LENGTH + 2] != '\0'; ++filesIndex)
  {
  }

  if (filesIndex < MAX_FILES)
  {
    int i, j, emptySector;
    // mengecek apakah banyak sector yang kosong mencukupi
    for (i = 0, emptySector = 0; i < SECTOR_SIZE && emptySector < *sectors; ++i)
    {
      if (map[i] == EMPTY)
      {
        ++emptySector;
      }
    }

    if (emptySector < *sectors)
    { // sector tidak cukup
      *sectors = INSUFFICIENT_SECTORS;
    }
    else
    { // sector cukup
      // cari sektor sektor_file yang kosong
      for (idxS = 0; idxS < MAX_SECTORS_FILESECTOR && sectorsFile[idxS*MAX_FILESECTOR] != '\0'; ++idxS)
      {
      }
      clear(files + filesIndex * FILES_ENTRY_LENGTH, FILES_ENTRY_LENGTH);
      clear(sectorsFile + idxS * MAX_FILESECTOR, MAX_FILESECTOR);
      files[filesIndex * FILES_ENTRY_LENGTH]  = idxP;
      files[filesIndex * FILES_ENTRY_LENGTH + 1] = idxS;
      // mengisi nama
      for (i = 0; i < MAX_FILENAME && path[i] != '\0'; ++i)
      {
        files[filesIndex * FILES_ENTRY_LENGTH + 2 + i] = filename[i];
      }
      // mengisi sector
      i = 0; emptySector = 0;
      while (i < SECTOR_SIZE && emptySector < *sectors)
      {
        if (map[i] == EMPTY)
        {
          clear(sectorBuffer, SECTOR_SIZE);
          for (j = 0; j < SECTOR_SIZE; ++j)
          {
            sectorBuffer[j] = buffer[emptySector * SECTOR_SIZE + j];
          }
          writeSector(sectorBuffer, i);
          sectorsFile[idxS * MAX_FILESECTOR + emptySector++] = i;
          map[i] = USED;
        }
        ++i; 
      }
      writeSector(map, MAP_SECTOR);
      writeSector(files, FILES_SECTOR);
      writeSector(files + SECTOR_SIZE, FILES_SECTOR+1);
      writeSector(sectorsFile, SECTORS_SECTOR);
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
  if (*success)
  {
    for (i = 0; i < 20 * 512; i++)
    {
      putInMemory(segment, i, buffer[i]);
    }
    launchProgram(segment);
  }
}
