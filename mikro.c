#include "lib/defines.h"
#include "lib/teks.h"
#include "lib/utils.h"
#include "lib/file.h"
#include "lib/extTeks.h"
#include "lib/video.h"

int len(char *buffer);
int countSector(char *buffer);
void readStringTeks(char *string, char *signal, int *len);
void ketSaveFile(char *filename, int sector);
void tulisFile(char *buffer, char *filename, int *sector, char parentIndex);
void render(char *title);
void clearKet();

int main(){
    char buffer[SECTOR_SIZE*MAX_FILESECTOR], filename[MAX_FILENAME];
    char title[(int)WIDTH+2];
    char isWrite = TRUE, signal, isValid, opt[MAX_FILENAME];
    int idx, idxTotal,success,sector,i;
    char parentIndex = 0xFF;

    clrscr();
    copyString("----------------    Selamat Datang di Mikro (Rivalnya Nano)!    ----------------",title,0);
    render(title);
    setPos(1, 0);
    idx = 0; idxTotal = 0;
    filename[0] = 0;
    while (isWrite){
        signal = 0;
        readStringTeks(buffer+idxTotal, &signal, &idx);
        idxTotal += idx;
        // printInt((int)signal);
        if (signal == CTRL_X){ // close file
            buffer[idxTotal] = '\0';
            success = FALSE;
            while (success!=TRUE)
            {
                isValid = FALSE;
                while (!isValid){
                    clearKet();
                    printString("Apakah ingin disimpan (Y/N): ");
                    readString(opt);
                    if (compare2String(opt,"Y") || compare2String(opt,"y") || 
                        compare2String(opt,"N") || compare2String(opt,"n")){
                            isValid = TRUE;
                        }
                    clearKet();
                }
                if (compare2String(opt,"Y") || compare2String(opt,"y")){
                    sector = countSector(buffer);
                    if (sector>MAX_FILESECTOR){
                        printString("Ukuran file terlalu besar\n\r");
                        success = TRUE;
                    }
                    else if (filename[0] != 0){  // sudah pernah disimpan
                        saveFile(buffer,&sector,findIdxFilename(filename,parentIndex));
                        ketSaveFile(filename, sector);
                        success = TRUE;
                    }
                    else{
                        while (success != TRUE){
                            success = sector;
                            printString("Nama file yang disimpan: ");
                            readString(filename);
                            tulisFile(buffer, filename, &success, parentIndex);
                        }
                    }
                }
                else{
                    success = TRUE;
                }
            }
            isWrite = FALSE;
        }
        else if (signal == CTRL_O) // open file
        {
            success = FALSE;
            while (success!=TRUE)
            {
                clearKet();
                printString("Nama file yang dibuka: ");
                readString(filename);
                readFile(buffer, filename, &success, parentIndex);
                if (success == FILE_NOT_FOUND){
                    printString("File tidak ada");
                }
                else{
                    printString(buffer);
                    idxTotal = len(buffer);
                }
            }
        }
        else if (signal == CTRL_S)  // save file
        {
            if (filename[0]!=0){
                sector = countSector(buffer);
                saveFile(buffer,&sector,findIdxFilename(filename,parentIndex));
            }
        }
        else
        {
            buffer[idxTotal++] = '\r';
            buffer[idxTotal++] = '\n';
        }
        
    }
}

int len(char *buffer){
    int i;
    for (i=0; buffer[i] != 0; ++i)
    {
    }
    return i;
}

int countSector(char *buffer){
    int cnt=0,cntRes=0,i=0;
    while (buffer[i]!='\0'){
        if (++cnt == SECTOR_SIZE)
        {
            ++cntRes;
        }
        ++i;
    }
    return ++cntRes;
}

void ketSaveFile(char *filename, int sector){
    if (sector==1){
        printString("File ");
        printString(filename);
        printString(" berhasil disimpan.\n\r");
    }
    else if (sector == FILE_HAS_EXIST)
    {
        printString("File sudah ada\n\r");
    }
    else if (sector == INSUFFICIENT_FILES)
    {
        printString("Tidak cukup menampung file\n\r");
    }
    else if (sector == INSUFFICIENT_SECTORS)
    {
        printString("Tidak cukup menyimpan isi file\n\r");
    }
    else if (sector == INVALID_FOLDER)
    {
        printString("Folder tidak valid\n\r");
    }
}

void tulisFile(char *buffer, char *filename, int *sector, char parentIndex){
    writeFile(buffer, filename, sector, parentIndex);
    ketSaveFile(filename, *sector);
}

void render(char *title){
    int i;

    printString(title);
    setPos(MAX_ROW+1, 0);
    for (i=0; i<=MAX_COL; ++i){
        printString("-");
    }
    printString("^X: Keluar      ^S: Menyimpan      ^O: Membuka");
}

void clearKet(){
    setPos(MAX_ROW+2, 0);
    clearLine();
    setPos(MAX_ROW+2, 0);
}