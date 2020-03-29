#ifndef __FOLDER_H__
#define __FOLDER_H__

/**
 * Membuat Folder Baru
 */
void createFolder();

/**
 * Menghapus folder dengan pathnya path dan sekarang sedang di parentIndex.
 * result akan bernilai:
 * 1, jika folder berhasil dihapus
 * -1, jika ada nama folder di path yang tidak ditemukan
 */
void deleteFolder(char *path, int *result, char parentIndex);

/**
 * content berisi filenames yang berada pada suatu parentIndex, count bernilai banyak file yang ada
 */
void listContent(char *content, int *count, char parentIndex, char *files);

/**
 * Menemukan idx dari suatu filename pada suatu parentIndex. Return -1 jika tidak ketemu
 */
int findIdxFilename(char *filename, char parentIndex, char *files);

/**
 * Menuju ke folder dengan pathnya path dan sekarang sedang di parentIndex.
 * result akan bernilai:
 * 1, jika folder berhasil pindah
 * -1, jika ada nama folder di path yang tidak ditemukan
 * -2, jika ternyata di pathnya ada yang berupa file (bukan directory/folder)
 * parentIndex akan bernilain posisi hasilnya. Jika gagal pindah, path akan terisi nama file penyebab
 * error.
 */
void goToFolder(char *path, int *result, char *parentIndex, char *files);

#endif