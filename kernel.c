/* Ini deklarasi fungsi */
void handleInterrupt21 (int AX, int BX, int CX, int DX);
void printString(char *string);
// void readString(char *string) {}
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
// void readFile(char *buffer, char *filename, int *success) {}
// void clear(char *buffer, int length) {} //Fungsi untuk mengisi buffer dengan 0
// void writeFile(char *buffer, char *filename, int *sectors) {}
// void executeProgram(char *filename, int segment, int *success) {}

int main() {
  printString("printString");

  while (1);
}

void printString(char *string) {
  int i = 0;
  while (string[i] != '\0') {
    interrupt(0x10, (0xE * 256) + string[i], 0, 0);
    i++;
  }
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

void readSector(char *buffer, int sector) {
  interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1,
            mod(div(sector, 18), 2) * 0x100);
}

void handleInterrupt21 (int AX, int BX, int CX, int DX) {
  switch (AX) {
    case 0x0:
      printString(BX);
      break;
    case 0x1:
      // readString(BX);
      break;
    case 0x2:
      // readSector(BX, CX);
      break;
    case 0x3:
      // writeSector(BX, CX);
      break;
    case 0x4:
      // readFile(BX, CX, DX);
      break;
    case 0x5:
      // writeFile(BX, CX, DX);
      break;
    case 0x6:
      // executeProgram(BX, CX, DX);
      break;
    default:
      // printString("Invalid interrupt");
      break;
  }
}
