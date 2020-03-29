#ifndef __FILE_H__
#define __FILE_H__

/**
 * Baca file
 * result bernilai:
 * 1, jika berhasil dibaca
 * -1, jika file tidak ada
 * buffer berisi isi file
 */
void readFile(char *buffer, char *path, int *result, char parentIndex);

/**
 * Tulis file
 * result bernilai:
 * 1, jika berhasil ditulis
 * -1, jika file sudah ada
 * -2, jika tidak cukup entri di files
 * -3, jika tidak cukup sektor kosong
 * -4, jika folder tidak valid
 * buffer berisi isi file yang akan disimpan
 */
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);


void deleteFile(char *path, int *result, char parentIndex);

#endif