#include"fileIO.h"

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

void clear(char *buffer, int length)
{
  int i;
  for (i = 0; i < length; i++)
  {
    buffer[i] = 0x00;
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

  if (compare2String(depan, "..")){
    if (parentIndex==0xFF) idxP = 0xFF;
    else idxP = files[parentIndex*FILES_ENTRY_LENGTH];
    isFound = TRUE;
  }
  else if (compare2String(depan, "."))
  {
    idxP = parentIndex;
    isFound = TRUE;
  }
  else
  {
    // cari yang sama
    idxP = 0;
    while(idxP<MAX_FILES && !isFound){
        if (files[idxP*FILES_ENTRY_LENGTH]==parentIndex && compare2String(files+idxP*FILES_ENTRY_LENGTH+2,depan)){
          isFound = TRUE;
        }
        else ++idxP;
    }
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
    for (j = i+1; path[j]!='\0'; ++j){
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
  // kalau ternyata files[idxP*FILES_ENTRY_LENGTH+1] berupa file, berarti ada duplikat
  else if (files[idxP*FILES_ENTRY_LENGTH+1] != 0xFF){
    *sectors = FILE_HAS_EXIST;
    return;
  }

  // mencari sektor files kosong
  for (filesIndex = 0; filesIndex < MAX_FILES && files[filesIndex * FILES_ENTRY_LENGTH + 2] != '\0'; ++filesIndex){}

  if (filesIndex < MAX_FILES)
  {
    int i, j, emptySector;
    // cari sektor sektor_file yang kosong
    for (idxS = 0; idxS < MAX_SECTORS_FILESECTOR && sectorsFile[idxS*MAX_FILESECTOR] != '\0'; ++idxS)
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
      files[filesIndex * FILES_ENTRY_LENGTH]  = idxP;
      files[filesIndex * FILES_ENTRY_LENGTH + 1] = idxS;
      // mengisi nama
      for (i = 0; i < MAX_FILENAME && path[i] != '\0'; ++i)
      {
        files[filesIndex * FILES_ENTRY_LENGTH + 2 + i] = filename[i];
      }
      // mengisi sector
      i = 0; emptySector = 0;
      while (emptySector < *sectors)
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

void deleteFile(char *path, int *result, char parentIndex){
    char files[2*SECTOR_SIZE];
    char sectors[SECTOR_SIZE];
    char filename[MAX_FILENAME];
    char map[SECTOR_SIZE];
    int iterSector, iterFile, idxP, idxS;

    readSector(files, FILES_SECTOR);
    readSector(files+SECTOR_SIZE, FILES_SECTOR+1);
    readSector(sectors, SECTORS_SECTOR);
    readSector(map, MAP_SECTOR);

    // index file nya
    idxP = searchRecurr(files, path, parentIndex, 0, filename);
    if (idxP == FILE_NOT_FOUND){
        *result = FILE_NOT_FOUND;
        return;
    }

    // sectornya
    idxS = files[idxP * FILES_ENTRY_LENGTH+1];
    iterSector = 0;
    while (iterSector < MAX_FILESECTOR)
    {
        map[sectors[idxS * MAX_FILESECTOR + iterSector]] = '\0';
        sectors[idxS * MAX_FILESECTOR + iterSector] = '\0';
        ++iterSector;
    }

    iterFile = 0;
    while (iterFile < FILES_ENTRY_LENGTH){
        files[idxP*FILES_ENTRY_LENGTH + iterFile] = '\0';  

    }

    writeSector(map, MAP_SECTOR);
    writeSector(files, FILES_SECTOR);
    writeSector(files + SECTOR_SIZE, FILES_SECTOR+1);
    writeSector(sectors, SECTORS_SECTOR);
}