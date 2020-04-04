#ifndef __EXTTEKS_H__
#define __EXTTEKS_H__

#define CTRL_O 0x0F
#define CTRL_S 0x13
#define CTRL_X 0x18
#define UP 0x4800
#define DOWN 0x5000
#define LEFT 0x4B00
#define RIGHT 0x4D00

/**
 * Membaca teks untuk program mikro
 */
void readStringTeks(char *string, char *signal, int *len);

/**
 * Mencari nRow baris sebelumnya dimulai indeks berapa
 */
int findIdxBefore(char *string, int nRow, int idxNow);

/**
 * Sigma panjang per baris sampai idxRow
 */
int sigmaLenPerRow(int *lenPerRow, int idxRow);
#endif