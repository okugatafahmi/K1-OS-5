#include "file.h"
#include "defines.h"
#include "utils.h"

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

  interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
  interrupt(0x21, 0x2, files+SECTOR_SIZE, FILES_SECTOR+1, 0);
  interrupt(0x21, 0x2, sectors, SECTORS_SECTOR, 0);
  
  idxP = searchRecurr(files, path, parentIndex, 0, filename);
  if (idxP == FILE_NOT_FOUND){
    *result = FILE_NOT_FOUND;
    return;
  }

  idxS = files[idxP * FILES_ENTRY_LENGTH+1];
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
  char files[2*SECTOR_SIZE], map[SECTOR_SIZE], sectorBuffer[SECTOR_SIZE], sectorsFile[SECTOR_SIZE], filename[MAX_FILENAME];
  int filesIndex, idxP, idxS;

  interrupt(0x21, 0x2, map, MAP_SECTOR, 0);
  interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
  interrupt(0x21, 0x2, files+SECTOR_SIZE, FILES_SECTOR+1, 0);
  interrupt(0x21, 0x2, sectorsFile, SECTORS_SECTOR, 0);
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
          interrupt(0x21, 0x3, sectorBuffer, i, 0);
          sectorsFile[idxS * MAX_FILESECTOR + emptySector++] = i;
          map[i] = USED;
        }
        ++i; 
      }
      interrupt(0x21, 0x3, map, MAP_SECTOR, 0);
      interrupt(0x21, 0x3, files, FILES_SECTOR, 0);
      interrupt(0x21, 0x3, files + SECTOR_SIZE, FILES_SECTOR+1, 0);
      interrupt(0x21, 0x3, sectorsFile, SECTORS_SECTOR, 0);
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

    interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
    interrupt(0x21, 0x2, files+SECTOR_SIZE, FILES_SECTOR+1, 0);
    interrupt(0x21, 0x2, sectors, SECTORS_SECTOR, 0);
    interrupt(0x21, 0x2, map, MAP_SECTOR, 0);

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

    interrupt(0x21, 0x3, map, MAP_SECTOR, 0);
    interrupt(0x21, 0x3, files, FILES_SECTOR, 0);
    interrupt(0x21, 0x3, files + SECTOR_SIZE, FILES_SECTOR+1, 0);
    interrupt(0x21, 0x3, sectors, SECTORS_SECTOR, 0);
}