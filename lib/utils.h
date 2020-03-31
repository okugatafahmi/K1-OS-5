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
 * Mensplit string input ke dalam argv. 1 baris argv berisi SECTOR_SIZE karakter
 * rowLen merupakan panjang 1 baris argv
 */
void split(char *input, char separator, char *argv, char *argc, int rowLen);

/**
 * Mensplit suatu path menjadi pathTo (path menuju file/folder) dan filename
 * (file/ folder yang merupakan nama terujung setelah '/' terakhir)
 */
void splitPath(char *path, char *pathTo, char *filename);

/**
 * Menemukan idx dari suatu filename pada suatu parentIndex. Return -1 (FILE_NOT_FOUND) jika tidak ketemu
 */
int findIdxFilename(char *filename, char parentIndex);

/**
 * Mengembalikan index sector file dari file ke-fileIdx
 */
char getIdxFileSector(int fileIdx);

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

/**
 * Menaruh argumen berikut ke ARGS_SECTOR
 */
void putArgs(char idxNow,char argc, char *argv);

/**
 * Mendapatkan argumen berikut dari ARGS_SECTOR
 */
void getArgs(char *idxNow,char *argc, char *argv);
#endif