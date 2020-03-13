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
#define EXIT_PROGRAM 2

void readString(char *string);
int commandType(char *command);
char compare2String(char* s1, char* s2);
void copyString(char* s1, char* s2);
void undo(char* command, char* history, int cntIsiHistory);
void clear(char *buffer, int length);
int findIdxFilename(char *filename, int parentIndex, char *files);
void setPath(char *path, int idxPathNow, int *iter, char *files);
void executeCD(char *path, int *idxPathNowReal, char *pathNow, char *files);

int main(){
	char isRun = TRUE;
	char success;
	char command[MAX_FILENAME*MAX_FILES];
	int type, i;
	int idxNext, parentIndex;
	int idxNow = 0xFF;
	char isFound;
	char pathNow[MAX_FILENAME*MAX_FILES];
	char files[2*SECTOR_SIZE];
	char history[3*MAX_FILENAME];

	int cntIsiHistory = 0;
	
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
		readString(command);

		// masukin ke history
		if (cntIsiHistory==3){ // udah penuh
			copyString(history+MAX_FILENAME, history); // dicopy dulu 2 item terakhir
			copyString(history+2*MAX_FILENAME,history + MAX_FILENAME);
			copyString(command,history+2*MAX_FILENAME);
		}
		else{
			copyString(command,history+cntIsiHistory*MAX_FILENAME);
			cntIsiHistory++;
		}

		type = commandType(command);
		switch (type)
		{
		case CD:
			executeCD(command+3, &idxNow, pathNow, files);
			break;
		
		default:
			i = findIdxFilename(command, 1, files);
			if (i == -1)
				interrupt(0x21, 0x0, "Invalid command\n\r", 0x0, 0x0);
			else
				interrupt(0x21, (0x1 << 8) | 0x6, command, 0x2000, &success);
			break;
		}

		// if (type==RUN_FILE){ // ./namafile
		// 	i = 2;
		// 	while(command[i]!='\0'){	// copy namafile yang akan di run
		// 		temp[i-2] = command[i];
		// 		i++;
		// 	}
		// 	temp[i-2] = '\0';
		// 	interrupt(0x21, idxNow, temp, 0x2000, &success);
		// 	if (success != 1)
		//     {
		//     	interrupt(0x21, 0x0, "Failed to execute file\n\r", 0, 0);
		//     }

		// }
		// else if (type==EXIT_PROGRAM){
		// 	isRun = FALSE;
		// }
		// else{
		// 	interrupt(0x21, 0x0, "Invalid command\n\r", 0x0, 0x0);
		// }
	}
}

void undo(char* command, char* history, int cntIsiHistory){
	if (cntIsiHistory==0){
		// do nothing
	}
	else{
		copyString(history+(cntIsiHistory-1)*MAX_FILENAME,command);
		interrupt(0x21, 0x0, command, 0, 0);
	}
}

void copyString(char* s1, char* s2){// copy s1 ke s2
	int i;
	for (i = 0; s1[i]!='\0'; ++i){
		s2[i] = s1[i];
	}
	s2[i] = '\0';
}

char compare2String(char* s1, char* s2){
  int i;
  for (i = 0; i<MAX_FILENAME; ++i){
    if (s1[i]=='\0' && s2[i]=='\0') return TRUE;
    if (s1[i]!=s2[i]) return FALSE;
  }
}


int commandType(char *command){
	if (command[0]=='c' && command[1]=='d' && command[2]==' ') return CD;
	else if (command[0]=='.' && command[1]=='/') return RUN_FILE;
	else if (compare2String("exit",command)) return EXIT_PROGRAM;
}

void readString(char *string)
{
  int i = 0;
  char input = 0;
  while (input != '\r')
  {
    input = interrupt(0x16, 0, 0, 0, 0);

    if (input == '\b')
    {
      if (i > 0)
      {
		interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
		interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
		interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
        i--;
      }
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

void clear(char *buffer, int length)
{
  int i;
  for (i = 0; i < length; i++)
  {
    buffer[i] = 0x00;
  }
}

int findIdxFilename(char *filename, int parentIndex, char *files){
	char isFound; 
	int idxFiles=0;

	isFound = FALSE;
	while(idxFiles<MAX_FILES && !isFound){
		if (files[idxFiles*FILES_ENTRY_LENGTH]==parentIndex && compare2String(files+idxFiles*FILES_ENTRY_LENGTH+2,filename)){
			isFound = TRUE;
		}
		else ++idxFiles;
	}
	if (isFound) return idxFiles;
	else return -1;
}

void setPath(char *path, int idxPathNow, int *iter, char *files){
	if (idxPathNow == 0xFF){
		path[0] = '/';
		*iter = 1;
	}
	else{
		int i = 0;
		setPath(path, files[idxPathNow*FILES_ENTRY_LENGTH], iter, files);
		while(files[idxPathNow*FILES_ENTRY_LENGTH + 2 + i]!='\0'){
			path[(*iter)] = files[idxPathNow*FILES_ENTRY_LENGTH + 2 + i];
			++(*iter);
			++i;
		}
	}
}

void executeCD(char *path, int *idxPathNowReal, char *pathNow, char *files){
	int iterPath=0,i=0, parentIndex, idxPathNow = *idxPathNowReal, idxPathNext;
	char front[MAX_FILENAME];

	// copy namafolder
	while(path[iterPath]!='\0'){
		if (path[iterPath]=='/'){
			front[i] = '\0';
			if (compare2String(front,"..")){
				// ke index parent
				if (idxPathNow != 0xFF) idxPathNow = files[idxPathNow*FILES_ENTRY_LENGTH];
			}
			else if (!compare2String(front,"."))
			{
				// go to folder yang di cd in
				idxPathNext = findIdxFilename(front, idxPathNow, files);

				if (idxPathNext != -1){
					// ganti jadi index baru
					idxPathNow = idxPathNext;
				}
				else{
					interrupt(0x21, 0x0, front, 0, 0);
					interrupt(0x21, 0x0, ": No such file or directory\n\r", 0, 0);
					return;
				}
			}
			// bersihkan lagi front
			clear(front, MAX_FILENAME);
			i = 0;
		}
		else {
			front[i] = path[iterPath];
			++i;
		}
		++iterPath;
	}
	front[i] = '\0';
	if (compare2String(front,"..")){
		// ke index parent
		if (idxPathNow != 0xFF) idxPathNow = files[idxPathNow*FILES_ENTRY_LENGTH];
	}
	else if (!compare2String(front,"."))
	{
		// go to folder yang di cd in
		idxPathNext = findIdxFilename(front, idxPathNow, files);

		if (idxPathNext != -1){
			// ganti jadi index baru
			idxPathNow = idxPathNext;
		}
		else{
			interrupt(0x21, 0x0, front, 0, 0);
			interrupt(0x21, 0x0, ": No such file or directory\n\r", 0, 0);
			return;
		}
	}
	if (*idxPathNowReal != idxPathNow){
		*idxPathNowReal = idxPathNow;
		clear(pathNow, MAX_FILENAME*MAX_FILES);
		setPath(pathNow, idxPathNow, 0, files);
	}
}