#include "folder.h"
#include "defines.h"
#include "utils.h"
#include "file.h"

void createFolder(char *path, int *result, char parentIndex)
{
	char sectorFolder[SECTOR_FILES_SIZE];
	char sectorFolderPath[SECTOR_SIZE];
	char folderName[FILES_ENTRY_LENGTH];
	int i = 0,j;
	
	interrupt(0x21, 0x2, sectorFolder, FILES_SECTOR, 0);
	interrupt(0x21, 0x2, sectorFolder + SECTOR_SIZE, FILES_SECTOR + 1, 0);
	
	clear(sectorFolderPath, SECTOR_SIZE);
	clear(folderName, FILES_ENTRY_LENGTH);
	splitPath(path, sectorFolderPath, folderName);

	goToFolder(sectorFolderPath, result, &parentIndex);

	// menyesuaikan error kode yang diinginkan
	if (*result != 1){
		if (*result == -1) *result = -3;
		else *result = -4;
		return;
	}

	// cek apakah folder sudah ada
	i = findIdxFilename(folderName, parentIndex);
	if (i != -1){
		*result = FILE_HAS_EXIST;
		return;
	}
	
	i=0;
	while ((i < MAX_FILES) && sectorFolder[i * FILES_ENTRY_LENGTH] != 0) i++;

	if (i == MAX_FILES) *result = INSUFFICIENT_FILES;
	else {
		sectorFolder[i * FILES_ENTRY_LENGTH] = parentIndex;
		sectorFolder[i * FILES_ENTRY_LENGTH + 1] = 0xFF;
		j = 0;
		while (folderName[j] != '\0')
		{
			sectorFolder[i * FILES_ENTRY_LENGTH + 2 + j] = folderName[j];
			j++;
		}
		sectorFolder[i * FILES_ENTRY_LENGTH + 2 + j] = '\0';
		interrupt(0x21, 0x03, sectorFolder, FILES_SECTOR, 0);
		interrupt(0x21, 0x03, sectorFolder + SECTOR_SIZE, FILES_SECTOR + 1, 0);
		*result = 1;
	}
}

void deleteFolder(char *path, int *result, char parentIndex)
{
	int idxFiles;
	char filename[MAX_FILENAME], files[SECTOR_FILES_SIZE];

	interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
	interrupt(0x21, 0x2, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);
	goToFolder(path, result, &parentIndex);

	if (*result == 1)
	{ // berhasil ditemukan
		// iterasi dan hapus setiap file yang ada di dalam folder
		for (idxFiles = 0; idxFiles < MAX_FILES; ++idxFiles)
		{
			// kalau di dalam foldernya
			if (files[idxFiles * FILES_ENTRY_LENGTH] == parentIndex)
			{
				copyString(files + idxFiles * FILES_ENTRY_LENGTH + 2, filename, 0);
				// interrupt(0x21, 0x0, filename, 0, 0);
				// kalau berupa folder, hapus lagi dalemnya
				if (files[idxFiles * FILES_ENTRY_LENGTH + 1] == 0xFF)
				{
					deleteFolder(filename, 0, parentIndex); // mungkin error yg di 0
				}
				// kalau file
				else
				{
					deleteFile(filename, 0, parentIndex);
				}
				// read sector lg karena dihapus
				interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
				interrupt(0x21, 0x2, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);
			}
		}
		clear(files + parentIndex * FILES_ENTRY_LENGTH, FILES_ENTRY_LENGTH);
		interrupt(0x21, 0x3, files, FILES_SECTOR, 0);
		interrupt(0x21, 0x3, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);
	}
}

void listContent(char *content, int *count, char parentIndex)
{
	int idxFiles, i;
	char files[SECTOR_FILES_SIZE];

	interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
	interrupt(0x21, 0x2, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);
	*count = 0;

	for (idxFiles = 0; idxFiles < MAX_FILES; ++idxFiles)
	{
		if (files[idxFiles * FILES_ENTRY_LENGTH] == parentIndex)
		{
			for (i = 0; i < MAX_FILESECTOR; ++i)
			{
				content[(*count) * FILES_ENTRY_LENGTH + i] = files[idxFiles * FILES_ENTRY_LENGTH + i];
			}
			++(*count);
		}
	}
}