#include "defines.h"
#include "folder.h"

int main() {
    char args[SECTOR_SIZE];
    char currentFolder;
    char argc;
    char argv[1][ARGS_LENGTH + 1];
    int succI;
  
    interrupt(0x21, 0x02, args, SECTOR_SIZE, 0);
    currentFolder = args[0];

    //Keluar
    interrupt(0x21, 0x20, currentFolder, 0, 0);
    interrupt(0x21, 0x7, ' ', 0, 0);
}
