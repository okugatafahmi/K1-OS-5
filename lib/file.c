#include "file.h"
#include "defines.h"
#include "utils.h"

void readFile(char *buffer, char *path, int *result, char parentIndex)
{
  interrupt(0x21, parentIndex<<8 | 0x4, buffer, path, result);
}


void writeFile(char *buffer, char *path, int *sectors, char parentIndex)
{
  interrupt(0x21, parentIndex<<8 | 0x5, buffer, path, sectors);
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
      *result = FILE_IS_DIRECTORY;
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