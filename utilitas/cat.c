#include"../lib/file.h"
#include"../lib/teks.h"
#include"../lib/math.h"

#define SECTOR_SIZE 512
#define SECTORS_SECTOR 0x103
#define MAX_FILENAME 14
#define MAX_FILESECTOR 16

void readSector(char *buffer, int sector)
{
  interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1,
            mod(div(sector, 18), 2) * 0x100);
}

int main(){
    char sectors[SECTOR_SIZE];
    char fileName[MAX_FILENAME];
    char buffer[SECTOR_SIZE * MAX_FILESECTOR];
    int result;
    int i;

    // baca argumen, copy argumen ke fileName
    readSector(sectors, 600);
    i = 600;
    while(sectors[i]!='\0'){
        fileName[i-600] = sectors[i];
        i++;
    }
    fileName[i-600] = '\0';
    
    // read file terus print 
    readFile(buffer, fileName, &result, 0xFF);
    printString(buffer);
}
