#include "file.h"
#include "defines.h"
#include "utils.h"

void readFile(char *buffer, char *path, int *result, char parentIndex)
{
  char files[SECTOR_FILES_SIZE];
  char sectors[SECTOR_SIZE];
  char pathTo[SECTOR_SIZE], filename[MAX_FILENAME];
  int iterSector, idxP=parentIndex, idxS;

  interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
  interrupt(0x21, 0x2, files+SECTOR_SIZE, FILES_SECTOR+1, 0);
  interrupt(0x21, 0x2, sectors, SECTORS_SECTOR, 0);
  
  splitPath(path, pathTo, filename);
  goToFolder(pathTo, result, &parentIndex);
  if (*result != 1){
    *result = FILE_NOT_FOUND;
    return;
  }
  idxP = findIdxFilename(filename, parentIndex);
  if (idxP == FILE_NOT_FOUND){
    *result = FILE_NOT_FOUND;
    return;
  }

  idxS = files[idxP * FILES_ENTRY_LENGTH+1];
  if (idxS == 0xFF){  // jika folder
    *result = -2;
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
  interrupt(0x21, 0x2, files+SECTOR_SIZE, FILES_SECTOR+1, 0);
  interrupt(0x21, 0x2, sectorsFile, SECTORS_SECTOR, 0);

  splitPath(path, pathTo, filename);
  goToFolder(pathTo, &result, &parentIndex);
  if (result != 1){
    *sectors = INVALID_FOLDER;
    return;
  }

  idxP = findIdxFilename(filename, parentIndex);
  if (idxP != -1){  // menemukan filename yang sama di parent index tsb
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
      *sectors = TRUE;
    }
  }
  else
  {
    *sectors = INSUFFICIENT_FILES;
  }
}

void deleteFile(char *path, int *result, char parentIndex){
    char files[SECTOR_FILES_SIZE];
    char sectors[SECTOR_SIZE];
    char pathTo[SECTOR_SIZE], filename[MAX_FILENAME];
    char map[SECTOR_SIZE], kosong[SECTOR_SIZE];
    int iterSector, iterFile, idxP, idxS;

    interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
    interrupt(0x21, 0x2, files+SECTOR_SIZE, FILES_SECTOR+1, 0);
    interrupt(0x21, 0x2, sectors, SECTORS_SECTOR, 0);
    interrupt(0x21, 0x2, map, MAP_SECTOR, 0);
    clear(kosong, SECTOR_SIZE);

    // index file nya
    // idxP = searchRecurr(files, path, parentIndex, 0, filename);
    splitPath(path, pathTo, filename);
    goToFolder(pathTo, result, &parentIndex);
    if (*result != 1){
        *result = FILE_NOT_FOUND;
        return;
    }

    idxP = findIdxFilename(filename,parentIndex);
    if (idxP == FILE_NOT_FOUND){
      *result = FILE_NOT_FOUND;
      return;
    }
    // sectornya
    idxS = files[idxP * FILES_ENTRY_LENGTH+1];
    if (idxS == 0xFF){  // jika folder
      *result = -2;
      return;
    }
    iterSector = 0;
    while (iterSector < MAX_FILESECTOR && sectors[idxS * MAX_FILESECTOR + iterSector] != '\0')
    {
      interrupt(0x21, 0x3, kosong, sectors[idxS * MAX_FILESECTOR + iterSector], 0);
      map[sectors[idxS * MAX_FILESECTOR + iterSector]] = '\0';
      sectors[idxS * MAX_FILESECTOR + iterSector] = '\0';
      ++iterSector;
    }

    iterFile = 0;
    while (iterFile < FILES_ENTRY_LENGTH){
      files[idxP*FILES_ENTRY_LENGTH + iterFile] = '\0';  
      ++iterFile;
    }

    interrupt(0x21, 0x3, map, MAP_SECTOR, 0);
    interrupt(0x21, 0x3, files, FILES_SECTOR, 0);
    interrupt(0x21, 0x3, files + SECTOR_SIZE, FILES_SECTOR+1, 0);
    interrupt(0x21, 0x3, sectors, SECTORS_SECTOR, 0);
}

void saveFile(char *buffer, char *sector, int idxFile){
  char files[SECTOR_FILES_SIZE], entryFile[FILES_ENTRY_LENGTH];

  interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
  interrupt(0x21, 0x2, files+SECTOR_SIZE, FILES_SECTOR+1, 0);
  copyString(files + idxFile*FILES_ENTRY_LENGTH, entryFile,0);
  deleteFile(entryFile+2, 0, entryFile[0]);
  writeFile(buffer, entryFile+2, sector, entryFile[0]);
}