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

char findLastChar(char *buffer, int *idx);
int countSector(char *buffer);

int main(){
    char buffer[SECTOR_SIZE*MAX_FILESECTOR], filename[SECTOR_SIZE*MAX_FILESECTOR];
    char isWrite = TRUE, lastC;
    int idx, idxTotal,success,sector;

    interrupt(0x21, 0x0, "masuk\n\r", 0, 0);
    idx = 0; idxTotal = 0;
    while (isWrite){
        interrupt(0x21, 0x1, buffer+idxTotal, 0, 0);
        lastC = findLastChar(buffer+idxTotal, &idx);
        idxTotal += idx;
        if (lastC == 24){ // close file
            buffer[idxTotal] = '\0';
            success = 0;
            while (success!=1)
            {
                sector = countSector(buffer);
                interrupt(0x21, 0x0, "\r\n\nNama file yang disimpan: ", 0, 0);
                interrupt(0x21, 0x1, filename, 0, 0);
                interrupt(0x21, (0x2 << 8) | 0x5, buffer, filename, &sector);
                if (sector>0){
                    interrupt(0x21, 0x0, "File ", 0,0);
                    interrupt(0x21, 0x0, filename, 0,0);
                    interrupt(0x21, 0x0, " berhasil disimpan.\n\r", 0,0);
                }
                else if (sector == FILE_HAS_EXIST)
                {
                    interrupt(0x21, 0x0, "File sudah ada", 0, 0);
                }
                else if (sector == INSUFFICIENT_FILES)
                {
                    interrupt(0x21, 0x0, "Tidak cukup menampung file", 0, 0);
                }
                else if (sector == INSUFFICIENT_SECTORS)
                {
                    interrupt(0x21, 0x0, "Tidak cukup menyimpan isi file", 0, 0);
                }
                else if (sector == INVALID_FOLDER)
                {
                    interrupt(0x21, 0x0, "Folder tidak valid", 0, 0);
                }
                isWrite = FALSE;
            }
        }
        else if (lastC == 15) // open file
        {
            success = 0;
            while (success!=1)
            {
                interrupt(0x21, 0x0, "\r\n\nNama file yang dibuka: ", 0, 0);
                interrupt(0x21, 0x1, filename, 0, 0);
                interrupt(0x21, (0x2 << 8) | 0x4, buffer, filename, &success);
                if (success == FILE_NOT_FOUND){
                    interrupt(0x21, 0x0, "File tidak ada", 0, 0);
                }
                else{
                    interrupt(0x21, 0x0, buffer, 0, 0);
                    findLastChar(buffer, &idxTotal);
                }
            }
        }
        else
        {
            buffer[idxTotal++] = '\r';
            buffer[idxTotal++] = '\n';
        }
        
    }
}

char findLastChar(char *buffer, int *idx){
    int i;
    for (i=0; buffer[i] != 24 && buffer[i] != 15 && buffer[i] != 0; ++i)
    {
    }
    *idx = i;
    return buffer[i];
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