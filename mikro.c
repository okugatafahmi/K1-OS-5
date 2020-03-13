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

int main(){
    char buffer[SECTOR_SIZE*MAX_FILESECTOR], opsi[SECTOR_SIZE*MAX_FILESECTOR];
    char isWrite = TRUE;
    int idx;

    interrupt(0x21, 0x0, "masuk\n\r", 0, 0);
    while (isWrite){
        interrupt(0x21, 0x1, buffer, 0, 0);
        if (findLastChar(buffer, &idx) == 14){
            buffer[idx] = '\0';
            interrupt(0x21, 0x0, "Nama file: ", 0, 0);
            interrupt(0x21, 0x1, buffer, 0, 0);
            isWrite = FALSE;
        }
    }
}

char findLastChar(char *buffer, int *idx){
    int i;
    for (i=0; buffer[i] != 14 || buffer[i] != 0; ++i)
    {
    }
    *idx = i;
    return buffer[i];
}