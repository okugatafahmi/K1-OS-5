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
void listContent(char *content, int *count, char parentIndex);

#endif