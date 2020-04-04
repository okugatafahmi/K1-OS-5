#include "lib/defines.h"
#include "lib/folder.h"

#define CD 0
#define RUN_FILE 1
#define LS 2
#define EXIT_PROGRAM 3

#define LEN_HISTORY 3

#include "lib/utils.h"

void readString(char *string, char *history, int cntIsiHistory);
int commandType(char *command);
void splitInput(char *input, char *command, char *argc, char *argv);
void setPath(char *path, int idxPathNow, int *iter, char *files);
void executeCD(char *path, char *idxPathNowReal, char *pathNow);

int main()
{
	char isRun = TRUE;
	char input[SECTOR_FILES_SIZE], command[MAX_FILENAME], argv[MAX_ARGS * ARGS_LENGTH], argc;
	int type, i, success;
	char idxNow = 0xFF; // idx folder sekarang
	char isFound;
	char pathNow[SECTOR_FILES_SIZE];
	char history[LEN_HISTORY * SECTOR_FILES_SIZE];
	char isAfterUndo = FALSE;
	char content[SECTOR_FILES_SIZE];

	int cntIsiHistory = 0;
	int cntContent;

	for (i = 0; i < 3 * MAX_FILENAME; ++i)
	{
		history[i] = '\0';
	}

	// path awal = root (/)
	pathNow[0] = '/';
	pathNow[1] = '\0';

	while (isRun)
	{
		interrupt(0x21, 0x0, "Tim Bentar:", 0, 0);
		interrupt(0x21, 0x0, pathNow, 0, 0);
		interrupt(0x21, 0x0, "$ ", 0, 0);
		readString(input, history, cntIsiHistory);

		// masukin ke history
		if (cntIsiHistory == LEN_HISTORY)
		{ // udah penuh
			for (i = 0; i < LEN_HISTORY - 1; ++i)
			{ // dicopy dulu LEN_HISTORY-1 item terakhir
				copyString(history + (i + 1) * SECTOR_FILES_SIZE, history + i * SECTOR_FILES_SIZE, 0);
			}
			copyString(input, history + (LEN_HISTORY - 1) * SECTOR_FILES_SIZE, 0);
		}
		else
		{
			copyString(input, history + cntIsiHistory * SECTOR_FILES_SIZE, 0);
			cntIsiHistory++;
		}

		splitInput(input, command, &argc, argv);
		putArgs(idxNow, argc, argv);
		type = commandType(command);
		switch (type)
		{
		case CD:
			if (argc > 1)
			{
				interrupt(0x21, 0x0, "cd: too many arguments\n\r", 0, 0);
			}
			else
			{
				executeCD(argv, &idxNow, pathNow);
			}
			break;
		case RUN_FILE:
			interrupt(0x21, (idxNow << 8) | 0x6, argv, 0x2000, &success);
			if (success != 1)
			{
				interrupt(0x21, 0x0, "Failed to execute file\n\r", 0, 0);
			}
			break;
		case LS:
			listContent(content, &cntContent, idxNow);
			while (cntContent--)
			{
				interrupt(0x21, 0x0, content + cntContent * MAX_FILESECTOR + 2, 0, 0);
				if (content[cntContent * MAX_FILESECTOR + 1] == 0xFF)
				{
					interrupt(0x21, 0x0, "/", 0, 0);
				}
				interrupt(0x21, 0x0, "\n\r", 0, 0);
			}
			break;
		case EXIT_PROGRAM:
			isRun = FALSE;
			break;
		default:
			interrupt(0x21, (0x1 << 8) | 0x6, command, 0x2000, &success);
			if (success == -1)
			{
				interrupt(0x21, 0x0, "Invalid command\n\r", 0x0, 0x0);
			}
			break;
		}
	}
}

int commandType(char *command)
{
	if (compare2String("cd", command))
		return CD;
	else if (compare2String(".", command))
		return RUN_FILE;
	else if (compare2String("ls", command))
		return LS;
	else if (compare2String("exit", command))
		return EXIT_PROGRAM;
	else
		return -1;
}

void readString(char *string, char *history, int cntIsiHistory)
{
	int i = 0, idxHistory = cntIsiHistory;
	char input = 0, temp[SECTOR_FILES_SIZE];
	char copyToTemp = FALSE;
	int ax, bx, cx, dx;
	while (input != '\r')
	{
		ax = 0;
		bx = 0;
		cx = 0;
		dx = 0;
		input = interruptEdit(0x16, &ax, &bx, &cx, &dx);
		// input = (char) inputReal;
		if (input == '\b')
		{
			if (i > 0)
			{
				interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
				interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
				interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
				string[--i] = '\0';
			}
		}
		else if ((ax == 0x4800 || ax == 0x5000))
		{
			if (cntIsiHistory == 0)
				continue;
			if (ax == 0x4800 && idxHistory > 0) // up arrow key
			{
				--idxHistory;
				if (idxHistory == cntIsiHistory - 1)
				{ // nyimpen command yang dimasukkan
					clear(temp, SECTOR_FILES_SIZE);
					copyToTemp = TRUE;
				}
			}
			else if (ax == 0x5000 && idxHistory < cntIsiHistory) // down arrow key
			{
				++idxHistory;
			}
			else
			{
				continue;
			}

			while (i--)
			{
				interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
				interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
				interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
				if (copyToTemp)
					temp[i] = string[i];
				string[i] = '\0';
			}
			copyToTemp = FALSE;
			if (idxHistory != cntIsiHistory)
			{
				copyString(history + idxHistory * SECTOR_FILES_SIZE, string, &i);
			}
			else
			{
				copyString(temp, string, &i);
			}
			interrupt(0x21, 0x0, string, 0, 0);
		}
		else
		{
			string[i] = input;
			interrupt(0x10, 0xE00 + input, 0, 0, 0);
			if (input != '\r')
				i++;
		}
	}
	string[i] = '\0';
	interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
	interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
}

void splitInput(char *input, char *command, char *argc, char *argv)
{
	int idx = 0;

	if (input[0] == '.' && input[1] == '/')
	{
		command[0] = '.';
		command[1] = '\0';
		idx = 1;
	}
	else
	{
		while (input[idx] != ' ' && input[idx] != '\0')
		{
			command[idx] = input[idx];
			++idx;
		}
		command[idx] = '\0';
	}
	// idx berada di posisi sebelum karakter pertama argv
	if (input[idx] != '\0')
	{
		while (input[idx] == ' ')
			++idx; // diposisi karakter pertama argv
		split(input + idx, ' ', argv, argc, ARGS_LENGTH);
	}
}

void setPath(char *path, int idxPathNow, int *iter, char *files)
{
	if (idxPathNow == 0xFF)
	{
		path[0] = '/';
		*iter = 1;
	}
	else
	{
		int i = 0;
		setPath(path, files[idxPathNow * FILES_ENTRY_LENGTH], iter, files);
		if (*iter != 1)
			path[(*iter)++] = '/';
		while (files[idxPathNow * FILES_ENTRY_LENGTH + 2 + i] != '\0')
		{
			path[(*iter)] = files[idxPathNow * FILES_ENTRY_LENGTH + 2 + i];
			++(*iter);
			++i;
		}
	}
}

void executeCD(char *path, char *idxPathNowReal, char *pathNow)
{
	int result;
	char idxPathNow = *idxPathNowReal, files[SECTOR_FILES_SIZE];

	interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
	interrupt(0x21, 0x2, files + SECTOR_SIZE, FILES_SECTOR + 1, 0);

	goToFolder(path, &result, &idxPathNow);

	if (result == INVALID_FOLDER)
	{
		interrupt(0x21, 0x0, path, 0, 0);
		interrupt(0x21, 0x0, ": No such file or directory\n\r", 0, 0);
	}
	else if (result == NOT_DIRECTORY)
	{
		interrupt(0x21, 0x0, path, 0, 0);
		interrupt(0x21, 0x0, ": Not a directory\n\r", 0, 0);
	}
	else
	{
		if (*idxPathNowReal != idxPathNow)
		{
			*idxPathNowReal = idxPathNow;
			clear(pathNow, SECTOR_FILES_SIZE);
			setPath(pathNow, idxPathNow, 0, files);
		}
	}
}