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
void printString(char *string);
int commandType(char *command);
char compare2String(char* s1, char* s2);
void copyString(char* s1, char* s2);
void undo(char* command, char* history, int cntIsiHistory);

int main(){
	char isRun = true;
	char success;
	char command[MAX_FILENAME];
	int type, i, idxPathNow;
	int idxNext, parentIndex;
	int idxNow = 0xFF;
	char isFound;
	char namaNow[MAX_FILENAME], temp[MAX_FILENAME], pathNow[MAX_FILENAME];
	char files[2*SECTOR_SIZE];
	char history[3*MAX_FILENAME];

	int cntIsiHistory = 0;
	
	for (i = 0; i<3*MAX_FILENAME; ++i){
		history[i] = '\0';
	}

	// nama root
	namaNow[0] = '/';
	namaNow[1] = '\0';

	// path awal = root (/)
	pathNow[0] = '/';
	pathNow[1] = '\0';
	idxPathNow = 1;
	
	readSector(files, FILES_SECTOR);
  	readSector(files+SECTOR_SIZE, FILES_SECTOR+1);

	while(isRun){
		printString(pathNow);
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
		if (type==CD){
			if (command[2]=='.'){// cd.. (asumsikan gacuma cd. doang dan input pasti benar)
				// ke index parent
				idxNow = files[idxNow*FILES_ENTRY_LENGTH];

				// kurangi pathnya (potong belakangnya)
				pathNow[--idxPathNow] = '\0';
				for (i = idxPathNow-1; pathNow[i]!='/'; --i){
					pathNow[i] = '\0';
				}

				// ganti nama folder
				for (i = 0; i<MAX_FILENAME; ++i){
					namafile[i] = files[idxNow*FILES_ENTRY_LENGTH+2+i];
				}
			}
			else{// cd namafolder
				// copy namafolder
				i = 3;
				while(command[i]!='\0'){
					temp[i-3] = command[i];
					i++;
				}
				temp[i-3] = '\0';

				// go to folder yang di cd in
				idxNext = 0;
				isFound = FALSE;
				parentIndex = idxNow;
				while(idxNext<MAX_FILES && !isFound){
			      if (files[idxNext*FILES_ENTRY_LENGTH]==parentIndex && compare2String(files+idxNext*FILES_ENTRY_LENGTH+2,temp)){
			        isFound = TRUE;
			      }
			      else ++idxNext;
				}

				// ganti jadi index baru
				idxNow = idxNext;

				// copy namafile baru
				for (i = 0; temp[i]!='\0'; ++i){
					namaNow[i] = temp[i];
					pathNow[idxPathNow++] = temp[i];
				}
				namaNow[i] = '\0';
				pathNow[idxPathNow++] = '/';
				pathNow[idxPathNow] = '\0';

			}
		}
		else if (type==RUN_FILE){ // ./namafile
			i = 2;
			while(command[i]!='\0'){	// copy namafile yang akan di run
				temp[i-2] = command[i];
				i++;
			}
			temp[i-2] = '\0';
			interrupt(0x21, idxNow, temp, 0x2000, &success);
			if (success != 1)
		    {
		    	interrupt(0x21, 0x0, "Failed to execute file\n\r", 0, 0);
		    }

		}
		else if (type==EXIT_PROGRAM){
			isRun = false;
		}
		else{
			printString("input salah");
		}
	}
}

void undo(char* command, char* history, int cntIsiHistory){
	if (cntIsiHistory==0){
		// do nothing
	}
	else{
		copyString(history+(cntIsiHistory-1)*MAX_FILENAME,command);
		printString(command);
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
	if (command[0]=='c' && command[1]=='d') return CD;
	else if (command[0]=='.' && command[1]=='/') return RUN_FILE;
	else if (compare2String("exit",command)) return EXIT_PROGRAM;
}

void printString(char *string)
{
  int i = 0;
  while (string[i] != '\0')
  {
    interrupt(0x10, (0xE * 256) + string[i], 0, 0);
    i++;
  }
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
      interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
      interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
      interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
      if (i > 0)
      {
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
