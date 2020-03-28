#include"fileIO.h"
#include"teks.h"

#define SECTOR_SIZE 512
#define SECTORS_SECTOR 0x103
#define MAX_FILENAME 14
#define MAX_FILESECTOR 16

int main(){
    char sectors[SECTOR_SIZE];
    char fileName[MAX_FILENAME];
    char buffer[SECTOR_SIZE * MAX_FILESECTOR];
    char result;
    int i;

    // baca argumen, copy argumen ke fileName
    readSector(sectors, SECTORS_SECTOR);
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