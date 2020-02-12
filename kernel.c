#define SECTOR_SIZE 512
#define DIR_SECTOR 2
#define DIR_ENTRY_LENGTH 32
#define MAX_FILENAME 12
#define MAX_FILESECTOR 20

/* Ini deklarasi fungsi */
void handleInterrupt21 (int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
int div(int,int);
int mod(int,int);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *filename, int *success);
void clear(char *buffer, int length); //Fungsi untuk mengisi buffer dengan 0
// void writeFile(char *buffer, char *filename, int *sectors) {}
void executeProgram(char *filename, int segment, int *success);

void printLogo();

int main() {
  char* line;
  printLogo();
  printString("printString\n\r"); 
  readString(line);
  printString("hasilnya:\n\r");
  printString(line);
  while (1);
}

void handleInterrupt21 (int AX, int BX, int CX, int DX) {
  switch (AX) {
    case 0x0:
      printString(BX);
      break;
    case 0x1:
      readString(BX);
      break;
    case 0x2:
      readSector(BX, CX);
      break;
    case 0x3:
      writeSector(BX, CX);
      break;
    case 0x4:
      readFile(BX, CX, DX);
      break;
    case 0x5:
      // writeFile(BX, CX, DX);
      break;
    case 0x6:
      executeProgram(BX, CX, DX);
      break;
    default:
      printString("Invalid interrupt");
      break;
  }
}

void printStringInMemory(char *string, int x, int y)
{
	int i = 0;
	while (string[i] != '\0' && string[i] != '\n')
	{
		int offset = 0x8000 + ((80 * y) + x) * 2;
		putInMemory(0xB000, offset, string[i]);
		i++;
		x++;
	}
}


void printLogo()
{
  printStringInMemory("* * * * * * * * * * * * * * * * * * * * * * * * * * * **", 0, 0);
  printStringInMemory("* ______                                               *", 0, 1);
  printStringInMemory("* | ___ \\  ______  _____  _________     __       ___    *",  0, 2);
  printStringInMemory("* | |_/ /  | ___   |     | ___   ___    /  \\    |   \\  *",  0, 3);
  printStringInMemory("* | ___ \\ | __    |     |    | |      /    \\   | | /   *",  0, 4);
  printStringInMemory("* | |_/ /  | ___   |     |    | |     |  ||  |   |   \\  *",  0, 5);
  printStringInMemory("* \\____/  |_____  |     |    | |     |      |   |    \\ *",  0, 6);
  printStringInMemory("* * * * * * * * * * * * * * * * * * * * * * * * * * * **",0, 7);
}

void printString(char *string) {
  int i = 0;
  while (string[i] != '\0') {
    interrupt(0x10, (0xE * 256) + string[i], 0, 0);
    i++;
  }
}

void readString(char *string){
  int i = 0;
  char input = 0;  
  while(input!= 0xD){
    input = interrupt(0x16, 0x0, 0x0, 0x0);
    interrupt(0x10, (0xE * 256) + input, 0, 0);
    if(input==0x8){                        
       if(i > 0){
          interrupt(0x10, 0xE*256+' ',0,0);
          interrupt(0x10, 0xE*256+input,0,0);
          i--;         
       }
    }
    else{
       string[i] = input;
       if (input == 0xD){
         interrupt(0x10,0xE*256+10,0x0,0x0);
       }
       i++;
    }
  }
  string[i] = 0x0;
}

int div(int a, int b) {
  int count = 0;
  while (b < a) {
    a -= b;
    count++;
  }
  return count;
}

int mod(int a, int b) {
  while (b < a) {
    a -= b;
  }
  return a;
}

void readSector(char *buffer, int sector) {
  interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1,
            mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector) {
  interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1,
            mod(div(sector, 18), 2) * 0x100);
}

void readFile(char *buffer, char *filename, int *success){
  char dir[SECTOR_SIZE];
  int iterDir,iterFileName;
  int iterLastByte, iterSector;
  int found,equal; // untuk boolean file
  // Membaca sector dir untuk membaca semua file
  readSector(dir,DIR_SECTOR);

  // Iterasi setiap file
  found=0;
  iterDir=0;
  while (!found && iterDir<SECTOR_SIZE)
  {
    iterFileName = 0;
    equal = 1;
    // Mencari yang nama file-nya sama
    while (equal && filename[iterFileName]!='\0' && iterFileName<MAX_FILENAME)
    {
      if (dir[iterDir+iterFileName] != filename[iterFileName])
      {
        equal = 0;
      }
      else
      {
        ++iterFileName;
      }
    }
    if (equal)
    {
      found = 1;
    }
    else
    {
      iterDir += DIR_ENTRY_LENGTH;
    }
    
  }
  
  if (found)
  {
    // menyimpan posisi awal sector file
    iterLastByte = iterDir + MAX_FILENAME;
    iterSector = 0;
    while (iterSector<MAX_FILESECTOR && dir[iterLastByte + iterSector]!=0)
    {
      readSector(buffer+iterSector*SECTOR_SIZE, dir[iterLastByte+iterSector]);
      ++iterSector;
    }
    
  }
  *success = found;
}

void clear(char *buffer, int length) {
  int i;
  for (i = 0; i < length; i++) {
    buffer[i] = 0x00;
  }
}

void executeProgram(char *filename, int segment, int *success) {
  int i;
  char buffer[512 * 20];
  readFile(&buffer, filename, success);
  if (*success) {
    for (i = 0; i < 20*512; i++) {
      putInMemory(segment, i, buffer[i])​;
    }
    launchProgram(segment);
  }
}
