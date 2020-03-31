#include "folder.h"
#include "defines.h"
#include "utils.h"
#include "file.h"

void createFolder(char *path, int *result, char parentIndex)
{
	char sectorFolder[SECTOR_SIZE];
	char sectorFolderPath[MAX_SECTORS_FILESECTOR * FILES_ENTRY_LENGTH];
	char folderName[FILES_ENTRY_LENGTH];
	int i = 0;
	
	interrupt(0x21, 0x02, sectorFolder, FILES_SECTOR, 0);
	while ((i < MAX_SECTORS_FILESECTOR) && sectorFolder[i * FILES_ENTRY_LENGTH + 1] != 0) i++;

	if (i == SECTOR_SIZE) *result = INSUFFICIENT_SECTORS;
	else {
		clear(sectorFolderPath, MAX_SECTORS_FILESECTOR * FILES_ENTRY_LENGTH);
		clear(folderName, FILES_ENTRY_LENGTH);
		splitPath(path, sectorFolderPath, folderName);
		int j = 0;
		while (folderName[j] != '\0')
		{
			sectorFolder[i * FILES_ENTRY_LENGTH + j + 1] = folderName[j];
			j++;
		}
		interrupt(0x21, 0x03, sectorFolder, FILES_SECTOR, 0);
		*result = 0;
	}
}

void deleteFolder(char *path, int *result, char parentIndex)
{
	int idxFiles;
	char filename[MAX_FILENAME], files[MAX_FILES * FILES_ENTRY_LENGTH];

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
	char files[MAX_FILES * FILES_ENTRY_LENGTH];

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