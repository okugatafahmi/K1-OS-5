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

#define LEN_HISTORY 3

void readString(char *string, char *history, int cntIsiHistory);
int commandType(char *command);
char compare2String(char* s1, char* s2);
void copyString(char* s1, char* s2, int *len);
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
	char history[LEN_HISTORY*MAX_FILENAME*MAX_FILES];
	char isAfterUndo = FALSE;

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

void copyString(char* s1, char* s2, int *len){// copy s1 ke s2
	int i;
	for (i = 0; s1[i]!='\0'; ++i){
		s2[i] = s1[i];
	}
	s2[i] = '\0';
	*len = i;
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
		if (*iter!=1) path[(*iter)++] = '/';
		while(files[idxPathNow*FILES_ENTRY_LENGTH + 2 + i]!='\0'){
			path[(*iter)] = files[idxPathNow*FILES_ENTRY_LENGTH + 2 + i];
			++(*iter);
			++i;
		}
	}
}

void executeCD(char *path, int *idxPathNowReal, char *pathNow, char *files){
	int iterPath=0,i=0, parentIndex, idxPathNow = *idxPathNowReal, idxPathNext;
	char front[MAX_FILENAME], isRoot = TRUE;

	clear(front,MAX_FILENAME);
	// copy namafolder
	while(path[iterPath]!='\0'){
		if (path[iterPath]=='/'){
			front[i] = '\0';
			if (compare2String(front,"..")){
				// ke index parent
				if (idxPathNow != 0xFF) idxPathNow = files[idxPathNow*FILES_ENTRY_LENGTH];
			}
			else if (front[0] == '\0' && isRoot){
				copyString("/",pathNow, 0);
				*idxPathNowReal = 0xFF;
				return;
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
			isRoot = FALSE;
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
	else if (!compare2String(front,".") && front[0]!='\0')
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