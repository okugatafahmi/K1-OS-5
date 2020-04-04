#ifndef __FOLDER_H__
#define __FOLDER_H__

/**
 * * Membuat folder dengan pathnya path dan sekarang sedang di parentIndex.
 * result akan bernilai:
 * 1, jika folder berhasil dihapus
 * FILE_HAS_EXIST, jika folder sudah ada
 * INSUFFICIENT_FILES, jika tidak cukup entri di files
 * INVALID_FOLDER, jika ada nama folder di path yang tidak ditemukan
 * NOT_DIRECTORY, jika ternyata di pathnya ada yang berupa file (bukan directory/folder)
 */
void createFolder(char *path, int *result, char parentIndex);

/**
 * Menghapus folder dengan pathnya path dan sekarang sedang di parentIndex.
 * result akan bernilai:
 * 1, jika folder berhasil dihapus
 * INVALID_FOLDER, jika ada nama folder di path yang tidak ditemukan
 * NOT_DIRECTORY, jika ternyata di pathnya ada yang berupa file (bukan directory/folder)
 * I.S. : path sudah benar ke suatu folder
 */
void deleteFolder(char *path, int *result, char parentIndex);

/**
 * Meng-copy isi folder yang berindeks idxSource
 * result akan bernilai:
 * 1, jika folder berhasil dicopy
 * INNSUFFICIENT_FILES, jika tidak cukup entri di files
 * INSUFFICIENT_SECTORS, jika tidak cukup sector file (ada copy file)
 * INVALID_FOLDER, jika ada nama folder di path yang tidak ditemukan
 * NOT_DIRECTORY, jika ternyata di pathnya ada yang berupa file (bukan directory/folder)
 * SAME_FOLDER, jika parentIndexnya sama
 * * I.S. : path sudah benar ke suatu folder dan parentIndexTarget ke suatu folder
 */
void copyFolder(char *path, char *filenameTarget, int *result, char parentIndexSource, char parentIndexTarget);

/**
 * content berisi filenames yang berada pada suatu parentIndex, count bernilai banyak file yang ada
 */
void listContent(char *content, int *count, char parentIndex);

#endif