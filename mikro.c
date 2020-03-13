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
    int idx,success;

    interrupt(0x21, 0x0, "masuk\n\r", 0, 0);
    idx = 0;
    while (isWrite){
        interrupt(0x21, 0x1, buffer+idx, 0, 0);
        lastC = findLastChar(buffer+idx, &idx);
        if (lastC == 24){
            buffer[idx] = '\0';
            success = 0;
            while (success!=1)
            {
                interrupt(0x21, 0x0, "\r\n\nNama file yang disimpan: ", 0, 0);
                interrupt(0x21, 0x1, filename, 0, 0);
                interrupt(0x21, (0x2 << 8) | 0x5, buffer, filename, countSector(buffer));
                isWrite = FALSE;
            }
        }
        else if (lastC == 15)
        {
            interrupt(0x21, 0x0, "\r\n\nNama file yang dibuka: ", 0, 0);
            interrupt(0x21, 0x1, filename, 0, 0);
            interrupt(0x21, (0x2 << 8) | 0x4, buffer, filename, countSector(buffer));
            isWrite = FALSE;
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
    int cnt=0;
    while (buffer[cnt++]!='\0'){}
    return cnt;
}