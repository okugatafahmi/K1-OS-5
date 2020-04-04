#ifndef __VIDEO_H__
#define __VIDEO_H__

#define HEIGHT 0x18 // tinggi window (posisi paling bawah) 24
#define WIDTH 0x4F  // lebar window (posisi paling kanan)  79
#define MIN_ROW 0x1
#define MAX_ROW 0x15
#define MIN_COL 0x0
#define MAX_COL 0x4F
#define LINE_SCROLL 0xA

/**
 * Membersihkan layar
 */
void clrscr();

/**
 * Membersihkan line sekarang
 */
void clearLine();

/**
 * Get posisi cursor
 */
void getPos(char *row, char *col);

/**
 * Set posisi cursor
 */
void setPos(char row, char col);

void scrollUp();

void scrollDown();

void printAtRow(char *string, char row);

#endif