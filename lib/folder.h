#ifndef __FOLDER_H__
#define __FOLDER_H__

/**
 * * Membuat folder dengan pathnya path dan sekarang sedang di parentIndex.
 * result akan bernilai:
 * 1, jika folder berhasil dihapus
 * -1, jika folder sudah ada
 * -2, jika tidak cukup entri di files
 * -3, jika ada nama folder di path yang tidak ditemukan
 * -4, jika ternyata di pathnya ada yang berupa file (bukan directory/folder)
 */
void createFolder(char *path, int *result, char parentIndex);

/**
 * Menghapus folder dengan pathnya path dan sekarang sedang di parentIndex.
 * result akan bernilai:
 * 1, jika folder berhasil dihapus
 * -1, jika ada nama folder di path yang tidak ditemukan
 * -2, jika ternyata di pathnya ada yang berupa file (bukan directory/folder)
 */
void deleteFolder(char *path, int *result, char parentIndex);

/**
 * content berisi filenames yang berada pada suatu parentIndex, count bernilai banyak file yang ada
 */
void listContent(char *content, int *count, char parentIndex);

#endif