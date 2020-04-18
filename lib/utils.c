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

void split(char *input, char separator, char *argv, char *argc, int rowLen)
{
	int idx = 0, row = 0, col = 0;
	char petik = '\0';

	while (input[idx] != '\0')
	{
		if (input[idx] == '\\')
		{
			argv[row * rowLen + col] = input[idx + 1];
			if (input[idx + 1] != '\0')
				++idx; // antisipasi error garing di plg blkg
		}
		else if (input[idx] == '"' || input[idx] == '\'')
		{
			if (petik == input[idx])
				petik = '\0'; // akhir petik
			else if (petik != '\0')
				argv[row * rowLen + col] = input[idx];
			else
				petik = input[idx];
		}
		else if (petik != '\0')
		{
			argv[row * rowLen + col] = input[idx];
		}
		else if (input[idx] == separator)
		{
			argv[row * rowLen + col] = '\0';
			while (input[idx + 1] == separator)
				++idx;
			if (input[idx + 1] != '\0')
			{
				col = -1;
				++row;
			}
		}
		else
		{
			argv[row * rowLen + col] = input[idx];
		}
		++col;
		++idx;
	}
	argv[row * rowLen + col] = '\0';
	*argc = (char)(row + 1);
}

void splitPath(char *path, char *pathTo, char *filename)
{
	int idxMid = -1, idx = 0, idxLastPathTo;

	while (path[idx] != '\0')
	{
		if (path[idx] == '/')
		{
			idxMid = idx;
		}
		++idx;
	}
	idx = 0;
	if (idxMid == 0)
	{
		pathTo[idx++] = '/';
	}
	else
	{
		for (idx = 0; idx < idxMid; ++idx)
		{
			pathTo[idx] = path[idx];
		}
	}
	pathTo[idx] = '\0';
	idxLastPathTo = idx;
	for (idx = idxMid + 1; path[idx] != '\0'; ++idx)
	{
		filename[idx - (idxMid + 1)] = path[idx];
	}
	filename[idx - (idxMid + 1)] = '\0';

	if (compare2String(filename, ".") || compare2String(filename, ".."))
	{
		idx = 0;
		pathTo[idxLastPathTo++] = '/';
		while (filename[idx] != '\0')
		{
			pathTo[idxLastPathTo++] = filename[idx++];
		}
		pathTo[idxLastPathTo] = '\0';
		filename[0] = '\0';
		filename[1] = '\0';
	}
}

int findIdxFilename(char *filename, char parentIndex)
{
	char isFound, files[SECTOR_FILES_SIZE];
	int idxFiles = 0;

	if (filename[0] == '\0')
		return FILE_NOT_FOUND;
	interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
	interrupt(0x21, 0x2, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);
	isFound = FALSE;
	while (idxFiles < MAX_FILES && !isFound)
	{
		if (files[idxFiles * FILES_ENTRY_LENGTH] == parentIndex && compare2String(files + idxFiles * FILES_ENTRY_LENGTH + 2, filename))
		{
			isFound = TRUE;
		}
		else
			++idxFiles;
	}
	if (isFound)
		return idxFiles;
	else
		return FILE_NOT_FOUND;
}

char getIdxFileSector(int fileIdx)
{
	char files[SECTOR_FILES_SIZE];

	interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
	interrupt(0x21, 0x2, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);
	return files[fileIdx * FILES_ENTRY_LENGTH + 1];
}

void setFileEntry(char *entry, int fileIdx)
{
	char files[SECTOR_FILES_SIZE];

	interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
	interrupt(0x21, 0x2, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);
	copyString(entry, files + fileIdx * FILES_ENTRY_LENGTH, 0);
	interrupt(0x21, 0x3, files, FILES_SECTOR, 0);
	interrupt(0x21, 0x3, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);
}

void goToFolder(char *path, int *result, char *parentIndex)
{
	int iterPath = 0, i = 0, idxPathNext;
	char front[MAX_FILENAME], files[SECTOR_FILES_SIZE];

	interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
	interrupt(0x21, 0x2, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);
	clear(front, MAX_FILENAME);
	*result = 1; // anggap berhasil dulu. Nanti kalau gagal baru di ubah
	if (path[0]=='\0') return;
	// copy namafolder
	while (path[iterPath] != '\0')
	{
		if ((path[iterPath] == '/') && (iterPath == 0))
		{
			*parentIndex = 0xFF;
		}
		else if (path[iterPath] == '/')
		{
			front[i] = '\0';
			if (compare2String(front, ".."))
			{
				// ke index parent
				if (*parentIndex != 0xFF)
					*parentIndex = files[*parentIndex * FILES_ENTRY_LENGTH];
			}
			else if (!compare2String(front, "."))
			{
				// go to folder yang di cd in
				idxPathNext = findIdxFilename(front, *parentIndex);

				if (idxPathNext != FILE_NOT_FOUND)
				{
					if (files[idxPathNext * FILES_ENTRY_LENGTH + 1] == 0xFF)
					{ // kalau dia folder, masuk
						// ganti jadi index baru
						*parentIndex = idxPathNext;
					}
					else
					{ // kalau ternyata berupa file
						copyString(front, path, 0);
						*result = NOT_DIRECTORY;
						return;
					}
				}
				else
				{
					copyString(front, path, 0);
					*result = INVALID_FOLDER;
					return;
				}
			}
			// bersihkan lagi front
			clear(front, MAX_FILENAME);
			i = 0;
		}
		else
		{
			front[i] = path[iterPath];
			++i;
		}
		++iterPath;
	}
	front[i] = '\0';
	if (compare2String(front, ".."))
	{
		// ke index parent
		if (*parentIndex != 0xFF)
			*parentIndex = files[*parentIndex * FILES_ENTRY_LENGTH];
	}
	else if (!compare2String(front, ".") && front[0] != '\0')
	{
		// go to folder yang di cd in
		idxPathNext = findIdxFilename(front, *parentIndex);

		if (idxPathNext != FILE_NOT_FOUND)
		{
			if (files[idxPathNext * FILES_ENTRY_LENGTH + 1] == 0xFF)
			{ // kalau dia folder, masuk
				// ganti jadi index baru
				*parentIndex = idxPathNext;
			}
			else
			{ // kalau ternyata berupa file
				copyString(front, path, 0);
				*result = NOT_DIRECTORY;
			}
		}
		else
		{
			copyString(front, path, 0);
			*result = INVALID_FOLDER;
		}
	}
}

void putArgs(char idxNow, char argc, char *argv)
{
	char args[SECTOR_FILES_SIZE];
	int i = 0, col;

	clear(args, SECTOR_FILES_SIZE);
	args[0] = idxNow;
	args[1] = argc;

	while (i < (int)argc)
	{
		for (col = 0; col < ARGS_LENGTH && argv[i * ARGS_LENGTH + col] != '\0'; ++col)
		{
			args[2 + i * ARGS_LENGTH + col] = argv[i * ARGS_LENGTH + col];
		}
		++i;
	}
	interrupt(0x21, 0x3, args, ARGS_SECTOR, 0);
	interrupt(0x21, 0x3, args + SECTOR_SIZE, ARGS_SECTOR + 1, 0);
}

void getArgs(char *idxNow, char *argc, char *argv)
{
	char args[SECTOR_FILES_SIZE];
	int i = 0, col;

	interrupt(0x21, 0x2, args, ARGS_SECTOR, 0);
	interrupt(0x21, 0x2, args + SECTOR_SIZE, ARGS_SECTOR + 1, 0);
	*idxNow = args[0];
	*argc = args[1];

	while (i < (int)*argc)
	{
		for (col = 0; col < ARGS_LENGTH && args[2 + i * ARGS_LENGTH + col] != '\0'; ++col)
		{
			argv[i * ARGS_LENGTH + col] = args[2 + i * ARGS_LENGTH + col];
		}
		argv[i * ARGS_LENGTH + col] = '\0';
		++i;
	}
}

int countSector(char *buffer)
{
	int cnt = 0, cntRes = 0, i = 0;
	while (buffer[i] != '\0')
	{
		if (++cnt == SECTOR_SIZE)
		{
			++cntRes;
		}
		++i;
	}
	return ++cntRes;
}

void executeProgram(char *filename, int segment, int *success, char parentIndex) {
	interrupt(0x21, (parentIndex << 8) | 0x6, filename, segment, success);
}