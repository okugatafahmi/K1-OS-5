#include "folder.h"
#include "defines.h"
#include "utils.h"

void createFolder(){

}

void deleteFolder(char *path, int *result, char parentIndex){
    
}

void listContent(char *content, int *count, char parentIndex, char *files){
	int idxFiles,i;
	*count=0;

	for (idxFiles=0; idxFiles<MAX_FILES; ++idxFiles){
		if (files[idxFiles*FILES_ENTRY_LENGTH] == parentIndex){
			for (i=0; i<MAX_FILESECTOR; ++i){
				content[(*count)*MAX_FILESECTOR+i] = files[idxFiles*MAX_FILESECTOR+i];
			}
			++(*count);
		}
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

void goToFolder(char *path, int *result, char *parentIndex, char *files){
	int iterPath=0,i=0, idxPathNext;
	char front[MAX_FILENAME], isRoot = TRUE;

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
					if (files[idxPathNext*MAX_FILESECTOR+1]==0xFF){ // kalau dia folder, masuk
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
			if (files[idxPathNext*MAX_FILESECTOR+1]==0xFF){ // kalau dia folder, masuk
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