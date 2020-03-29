#ifndef __UTILS_H__
#define __UTILS_H__

/**
 * Meng-clear array buffer
 */
void clear(char *buffer, int length);

/**
 * Meng-copy string s1 ke s2. len akan menyimpan panjang string tersebut
 */
void copyString(char *s1, char *s2, int *len);

/**
 * Membandingkan 2 string (array of char).
 * Return TRUE jika keduanya sama
 */
char compare2String(char *s1, char *s2);

/**
 * Menemukan idx dari suatu filename pada suatu parentIndex. Return -1 jika tidak ketemu
 */
int findIdxFilename(char *filename, char parentIndex);

/**
 * Menuju ke folder dengan pathnya path dan sekarang sedang di parentIndex.
 * result akan bernilai:
 * 1, jika folder berhasil pindah
 * -1, jika ada nama folder di path yang tidak ditemukan
 * -2, jika ternyata di pathnya ada yang berupa file (bukan directory/folder)
 * parentIndex akan bernilain posisi hasilnya. Jika gagal pindah, path akan terisi nama file penyebab
 * error.
 */
void goToFolder(char *path, int *result, char *parentIndex);

#endif