#include "folder.h"
#include "defines.h"
#include "utils.h"
#include "file.h"

void createFolder(){

}

void deleteFolder(char *path, int *result, char parentIndex){
    int idxFiles;
	char filename[MAX_FILENAME], files[MAX_FILES*FILES_ENTRY_LENGTH];

	interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
  	interrupt(0x21, 0x2, files+SECTOR_SIZE, FILES_SECTOR+1, 0);
	goToFolder(path,result,&parentIndex,files);

	if (*result==1){ // berhasil ditemukan
		// iterasi dan hapus setiap file yang ada di dalam folder
		for (idxFiles=0; idxFiles<MAX_FILES; ++idxFiles){
			// kalau di dalam foldernya
			if (files[idxFiles*FILES_ENTRY_LENGTH] == parentIndex){
				copyString(files+idxFiles*FILES_ENTRY_LENGTH+2, filename, 0);
				// interrupt(0x21, 0x0, filename, 0, 0);
				// kalau berupa folder, hapus lagi dalemnya
				if (files[idxFiles*FILES_ENTRY_LENGTH+1] == 0xFF){
					deleteFolder(filename, 0, parentIndex); // mungkin error yg di 0
				}
				// kalau file
				else{
					deleteFile(filename,0,parentIndex);
				}
				// read sector lg karena dihapus
				interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
  				interrupt(0x21, 0x2, files+SECTOR_SIZE, FILES_SECTOR+1, 0);
			}
		}
		clear(files+parentIndex*FILES_ENTRY_LENGTH,FILES_ENTRY_LENGTH);
		interrupt(0x21, 0x3, files, FILES_SECTOR, 0);
  		interrupt(0x21, 0x3, files+SECTOR_SIZE, FILES_SECTOR+1, 0);
	}
}

void listContent(char *content, int *count, char parentIndex){
	int idxFiles,i;
	char files[MAX_FILES*FILES_ENTRY_LENGTH];

	interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
  	interrupt(0x21, 0x2, files+SECTOR_SIZE, FILES_SECTOR+1, 0);
	*count=0;

	for (idxFiles=0; idxFiles<MAX_FILES; ++idxFiles){
		if (files[idxFiles*FILES_ENTRY_LENGTH] == parentIndex){
			for (i=0; i<MAX_FILESECTOR; ++i){
				content[(*count)*FILES_ENTRY_LENGTH+i] = files[idxFiles*FILES_ENTRY_LENGTH+i];
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