// loadFile.c
// Michael Black, 2007
//
// Loads a file into the file system
// This should be compiled with gcc and run outside of the OS

#include <stdio.h>
#define SECTOR_SIZE 512
void main(int argc, char* argv[]) {
  int i;

  if (argc < 2) {
    printf("Specify file name to load\n");
    return;
  }

  // open the source file
  FILE* loadFil;
  loadFil = fopen(argv[1], "r");
  if (loadFil == 0) {
    printf("File not found\n");
    return;
  }

  // open the floppy image
  FILE* floppy;
  floppy = fopen("system.img", "r+");
  if (floppy == 0) {
    printf("system.img not found\n");
    return;
  }

  // load the disk map
  char map[SECTOR_SIZE];
  fseek(floppy, SECTOR_SIZE * 0x100, SEEK_SET);
  for (i = 0; i < SECTOR_SIZE; i++) map[i] = fgetc(floppy);

  // load the directory
  char dir[SECTOR_SIZE * 2];
  fseek(floppy, SECTOR_SIZE * 0x101, SEEK_SET);
  for (i = 0; i < SECTOR_SIZE * 2; i++) dir[i] = fgetc(floppy);

  // load sector
  char sector[SECTOR_SIZE];
  fseek(floppy, SECTOR_SIZE * 0x103, SEEK_SET);
  for (i = 0; i < SECTOR_SIZE; i++) sector[i] = fgetc(floppy);
  
  // find a free entry in the directory
  for (i = 0; i < SECTOR_SIZE * 2; i = i + 0x10)
    if (dir[i] == 0 ) break;
  if (i == SECTOR_SIZE * 2) {
    printf("Not enough room in directory\n");
    return;
  }
  int dirindex = i;

  // fill the name field with 00s first
  for (i = 0; i < 12 + 2; i++) dir[dirindex + i + 2] = 0x00;
  // copy the name over
  for (i = 0; i < 12 + 2; i++) {
    if (argv[1][i] == 0) break;
    dir[dirindex + i + 2] = argv[1][i];
  }

  // find empty sector file
  for (i = 0; i < 32; i++)
  {
    if (sector[i * 16] == 0) break;
  }
  if (i == 32) {
    printf("Not enough room in directory entry\n");
    return;
  }
  dir[dirindex + 1] = i;

  for (i=0; argv[1][i]!='\0' && argv[1][i]!='.'; ++i){}
  if (argv[1][i]=='\0') dir[dirindex] = 0x1; // put at bin folder
  else dir[dirindex] = 0xFF;
  int sectindex = i;

  // find free sectors and add them to the file
  int sectcount = 0;
  while (!feof(loadFil)) {
    if (sectcount == 16) {
      printf("Not enough space in directory entry for file\n");
      return;
    }

    // find a free map entry
    for (i = 0; i < 256; i++)
      if (map[i] == 0) break;
    if (i == 256) {
      printf("Not enough room for file\n");
      return;
    }

    // mark the map entry as taken
    map[i] = 0xFF;
    

    // mark the sector in the directory entry
    sector[sectindex * 16 + sectcount] = i;
    sectcount++;

    printf("Loaded %s to sector %d\n", argv[1], i);

    // move to the sector and write to it
    fseek(floppy, i * SECTOR_SIZE, SEEK_SET);
    for (i = 0; i < SECTOR_SIZE; i++) {
      if (feof(loadFil)) {
        fputc(0x0, floppy);
        break;
      } else {
        char c = fgetc(loadFil);
        fputc(c, floppy);
      }
    }
  }

  // write the map and directory back to the floppy image
  fseek(floppy, SECTOR_SIZE * 0x100, SEEK_SET);
  for (i = 0; i < SECTOR_SIZE; i++) fputc(map[i], floppy);

  fseek(floppy, SECTOR_SIZE * 0x101, SEEK_SET);
  for (i = 0; i < SECTOR_SIZE * 2; i++) fputc(dir[i], floppy);

  fseek(floppy, SECTOR_SIZE * 0x103, SEEK_SET);
  for (i = 0; i < SECTOR_SIZE; i++) fputc(sector[i], floppy);

  fclose(floppy);
  fclose(loadFil);
}
