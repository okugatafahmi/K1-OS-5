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

#endif