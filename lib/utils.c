#include "utils.h"
#include "defines.h"

void clear(char *buffer, int length)
{
	int i;
	for (i = 0; i < length; i++)
	{
		buffer[i] = 0x00;
	}
}

void copyString(char *s1, char *s2, int *len)
{
	int i;
	for (i = 0; s1[i] != '\0'; ++i)
	{
		s2[i] = s1[i];
	}
	s2[i] = '\0';
	*len = i;
}

char compare2String(char *s1, char *s2)
{
	int i;
	for (i = 0; i < SECTOR_SIZE; ++i)
	{
		if (s1[i] == '\0' && s2[i] == '\0')
			return TRUE;
		if (s1[i] != s2[i])
			return FALSE;
	}
}

int findIdxFilename(char *filename, char parentIndex, char *files){
	char isFound; 
	int idxFiles=0;

	isFound = FALSE;
	while(idxFiles<MAX_FILES && !isFound){
		if (files[idxFiles*FILES_ENTRY_LENGTH]==parentIndex && compare2String(files+idxFiles*FILES_ENTRY_LENGTH+2,filename)){
			isFound = TRUE;
		}
		else ++idxFiles;
	}
	if (isFound) return idxFiles;
	else return -1;
}

void goToFolder(char *path, int *result, char *parentIndex){
	int iterPath=0,i=0, idxPathNext;
	char front[MAX_FILENAME], isRoot = TRUE, files[2*SECTOR_SIZE];

	interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
    interrupt(0x21, 0x2, files+SECTOR_SIZE, FILES_SECTOR+1, 0);
	clear(front,MAX_FILENAME);
	*result = 1; // anggap berhasil dulu. Nanti kalau gagal baru di ubah
	// copy namafolder
	while(path[iterPath]!='\0'){
		if (path[iterPath]=='/'){
			front[i] = '\0';
			if (compare2String(front,"..")){
				// ke index parent
				if (*parentIndex != 0xFF) *parentIndex = files[*parentIndex*FILES_ENTRY_LENGTH];
			}
			else if (front[0] == '\0' && isRoot){
				*parentIndex = 0xFF;
				return;
			}
			else if (!compare2String(front,"."))
			{
				// go to folder yang di cd in
				idxPathNext = findIdxFilename(front, *parentIndex, files);

				if (idxPathNext != -1){
					if (files[idxPathNext*FILES_ENTRY_LENGTH+1]==0xFF){ // kalau dia folder, masuk
						// ganti jadi index baru
						*parentIndex = idxPathNext;
					}
					else{ // kalau ternyata berupa file
						copyString(front,path,0);
						*result = -2;
						return;
					}
				}
				else{
					copyString(front,path,0);
					*result = -1;
					return;
				}
			}
			isRoot = FALSE;
			// bersihkan lagi front
			clear(front, MAX_FILENAME);
			i = 0;
		}
		else {
			front[i] = path[iterPath];
			++i;
		}
		++iterPath;
	}
	front[i] = '\0';
	if (compare2String(front,"..")){
		// ke index parent
		if (*parentIndex != 0xFF) *parentIndex = files[*parentIndex*FILES_ENTRY_LENGTH];
	}
	else if (!compare2String(front,".") && front[0]!='\0')
	{
		// go to folder yang di cd in
		idxPathNext = findIdxFilename(front, *parentIndex, files);

		if (idxPathNext != -1){
			if (files[idxPathNext*FILES_ENTRY_LENGTH+1]==0xFF){ // kalau dia folder, masuk
				// ganti jadi index baru
				*parentIndex = idxPathNext;
			}
			else{ // kalau ternyata berupa file
				copyString(front,path,0);
				*result = -2;
			}
		}
		else{
			copyString(front,path,0);
			*result = -1;
		}
	}
}