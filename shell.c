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
#define CD 0
#define RUN_FILE 1
#define LS 2
#define EXIT_PROGRAM 3

#define LEN_HISTORY 3
#define ARGS_SECTOR 600

#include "lib/utils.h"
#include "lib/folder.h"

void readString(char *string, char *history, int cntIsiHistory);
int commandType(char *command);
void setPath(char *path, int idxPathNow, int *iter, char *files);
void executeCD(char *path, char *idxPathNowReal, char *pathNow, char *files);

int main(){
	char isRun = TRUE;
	char success;
	char command[MAX_FILENAME*MAX_FILES];
	int type, i;
	char idxNow = 0xFF;   // idx folder sekarang
	char isFound;
	char pathNow[MAX_FILENAME*MAX_FILES];
	char files[2*SECTOR_SIZE];
	char history[LEN_HISTORY*MAX_FILENAME*MAX_FILES];
	char isAfterUndo = FALSE;
	char content[MAX_FILES*MAX_FILESECTOR];

	int cntIsiHistory = 0;
	int cntContent;
	
	for (i = 0; i<3*MAX_FILENAME; ++i){
		history[i] = '\0';
	}

	// path awal = root (/)
	pathNow[0] = '/';
	pathNow[1] = '\0';
	
	interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
  	interrupt(0x21, 0x2, files+SECTOR_SIZE, FILES_SECTOR+1, 0);

	while(isRun){
		interrupt(0x21, 0x0, "Tim Bentar:", 0, 0);
		interrupt(0x21, 0x0, pathNow, 0, 0);
		interrupt(0x21, 0x0, "$ ", 0, 0);
		readString(command, history, cntIsiHistory);

		// masukin ke history
		if (cntIsiHistory==LEN_HISTORY){ // udah penuh
			for (i=0;i<LEN_HISTORY-1; ++i){  // dicopy dulu LEN_HISTORY-1 item terakhir
				copyString(history+(i+1)*MAX_FILENAME*MAX_FILES,history + i*MAX_FILENAME*MAX_FILES,0);
			}
			copyString(command,history+(LEN_HISTORY-1)*MAX_FILENAME*MAX_FILES,0);
		}
		else{
			copyString(command,history+cntIsiHistory*MAX_FILENAME*MAX_FILES,0);
			cntIsiHistory++;
		}

		type = commandType(command);
		switch (type)
		{
		case CD:
			executeCD(command+3, &idxNow, pathNow, files);
			break;
		case RUN_FILE:
			interrupt(0x21, (idxNow<<8) | 0x6, command+2, 0x2000, &success);
			if (success != 1)
		    {
		    	interrupt(0x21, 0x0, "Failed to execute file\n\r", 0, 0);
		    }
			break;
		case LS:
			listContent(content,&cntContent,idxNow,files);
			while (cntContent--){
				interrupt(0x21, 0x0, content+cntContent*MAX_FILESECTOR+2,0,0);
				if (content[cntContent*MAX_FILESECTOR+1]==0xFF){
					interrupt(0x21, 0x0, "/", 0, 0);
				}
				interrupt(0x21, 0x0, "\n\r", 0, 0);
			}
			break;
		case EXIT_PROGRAM:
			isRun = FALSE;
			break;
		default:
			i = findIdxFilename(command, 1, files);
			if (i == -1)
				interrupt(0x21, 0x0, "Invalid command\n\r", 0x0, 0x0);
			else
				interrupt(0x21, (0x1 << 8) | 0x6, command, 0x2000, &success);
			break;
		}
	}
}

int commandType(char *command){
	if (command[0]=='c' && command[1]=='d' && command[2]==' ') return CD;
	else if (command[0]=='.' && command[1]=='/') return RUN_FILE;
	else if (compare2String("ls",command)) return LS;
	else if (compare2String("exit",command)) return EXIT_PROGRAM;
	else return -1;
}

void readString(char *string, char *history, int cntIsiHistory)
{
  int i = 0, inputReal, idxHistory=cntIsiHistory;
  char input = 0, temp[MAX_FILENAME*MAX_FILES];
  char copyToTemp = FALSE;
  while (input != '\r')
  {
    inputReal = interrupt(0x16, 0, 0, 0, 0);
	input = (char) inputReal;
    if (input == '\b')
    {
      if (i > 0)
      {
		interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
		interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
		interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
        string[--i] = '\0';
      }
    }
	else if ((inputReal == 0x4800 || inputReal == 0x5000))
	{
		if (cntIsiHistory==0) continue;
		if (inputReal == 0x4800 && idxHistory>0) // up arrow key
		{
			--idxHistory;
			if (idxHistory==cntIsiHistory-1) {	// nyimpen command yang dimasukkan
				clear(temp, MAX_FILENAME*MAX_FILES);
				copyToTemp = TRUE;
			}
		}
		else if (inputReal == 0x5000 && idxHistory<cntIsiHistory) // down arrow key
		{
			++idxHistory;
		}
		else
		{
			continue;
		}


		while (i--)
		{
			interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
			interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
			interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
			if (copyToTemp) temp[i] = string[i];
			string[i] = '\0';
		}
		copyToTemp = FALSE;
		if (idxHistory!=cntIsiHistory) 
		{
			copyString(history+idxHistory*MAX_FILENAME*MAX_FILES, string, &i);
		}
		else
		{
			copyString(temp, string, &i);
		}
		interrupt(0x21, 0x0, string, 0, 0);
	}
    else
    {
      string[i] = input;
      interrupt(0x10, 0xE00 + input, 0, 0, 0);
      if (input != '\r')
        i++;
    }
  }
  string[i] = '\0';
  interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
  interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
}

void setPath(char *path, int idxPathNow, int *iter, char *files){
	if (idxPathNow == 0xFF){
		path[0] = '/';
		*iter = 1;
	}
	else{
		int i = 0;
		setPath(path, files[idxPathNow*FILES_ENTRY_LENGTH], iter, files);
		if (*iter!=1) path[(*iter)++] = '/';
		while(files[idxPathNow*FILES_ENTRY_LENGTH + 2 + i]!='\0'){
			path[(*iter)] = files[idxPathNow*FILES_ENTRY_LENGTH + 2 + i];
			++(*iter);
			++i;
		}
	}
}

void executeCD(char *path, char *idxPathNowReal, char *pathNow, char *files){
	int result;
	char idxPathNow = *idxPathNowReal;
	goToFolder(path, &result, &idxPathNow, files);

	if (result==-1){
		interrupt(0x21, 0x0, path, 0, 0);
		interrupt(0x21, 0x0, ": No such file or directory\n\r", 0, 0);
		return;
	}
	else if (result==-2){
		interrupt(0x21, 0x0, path, 0, 0);
		interrupt(0x21, 0x0, ": Not a directory\n\r", 0, 0);
		return;
	}

	if (*idxPathNowReal != idxPathNow){
		*idxPathNowReal = idxPathNow;
		clear(pathNow, MAX_FILENAME*MAX_FILES);
		setPath(pathNow, idxPathNow, 0, files);
	}
}