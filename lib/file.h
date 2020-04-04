#ifndef __FILE_H__
#define __FILE_H__

/**
 * Baca file
 * result bernilai:
 * 1, jika berhasil dibaca
 * FILE_NOT_FOUND, jika file tidak ada
 * FILE_IS_FOLDER, jika ternyata filenya berupa folder
 * buffer berisi isi file
 */
void readFile(char *buffer, char *path, int *result, char parentIndex);

/**
 * Tulis file
 * result bernilai:
 * 1, jika berhasil ditulis
 * FILE_HAS_EXIST, jika file sudah ada
 * INSUFFICIENT_FILES, jika tidak cukup entri di files
 * INSUFFICIENT_SECTORS, jika tidak cukup sektor kosong
 * INVALID_FOLDER, jika folder tidak valid
 * buffer berisi isi file yang akan disimpan
 */
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);

/**
 * Hapus file
 * result bernilai:
 * 1, jika berhasil dihapus
 * FILE_NOT_FOUND, jika file tidak ada
 * FILE_IS_DIRECTORY, jika ternyata filenya berupa folder
 * I.S, : path ke suatu file
 */
void deleteFile(char *path, int *result, char parentIndex);

/**
 * Menyimpan file
 * * result bernilai:
 * 1, jika berhasil disimpan
 * INSUFFICIENT_SECTORS, jika tidak cukup sektor kosong
 */
void saveFile(char *buffer, char *sector, int idxFile);
#endif