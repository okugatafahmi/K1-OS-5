#include "lib/defines.h"
#include "lib/utils.h"
#include "lib/teks.h"
#include "lib/folder.h"
#include "lib/file.h"

void succeed(char *source, char *dest);
void notFound(char *path);
void notDir(char *path);
void targetNotDir(char *path);
void rename(char parentIndex, char idxSector, char *filename, int idxFile);
int getIdxTarget(char *argTarget, char parentIndexTarget);

int main()
{
    char parentIndex, parentIndexSource, parentIndexTarget, argc, argv[MAX_ARGS * ARGS_LENGTH];
    char pathSource[ARGS_LENGTH], filenameSource[MAX_FILENAME];
    char pathTarget[ARGS_LENGTH], filenameTarget[MAX_FILENAME];
    char fileEntry[FILES_ENTRY_LENGTH];
    char idxSectorSource, idxSectorTarget, *argSource, *argTarget;
    int i, idxSource, idxTarget, success;

    getArgs(&parentIndex, &argc, argv);
    parentIndexSource = parentIndex;
    parentIndexTarget = parentIndex;
    if (argc < 2)
    {
        printString("mv: missing destination file operand after '");
        printString(argv);
        printString("'\n\r");
    }
    else if (argc == 2)
    {
        splitPath(argv, pathSource, filenameSource);
        splitPath(argv + ARGS_LENGTH, pathTarget, filenameTarget);
        // cari path source filenya
        goToFolder(pathSource, &success, &parentIndexSource);
        if (success == 1)
        {
            // cari index file source
            idxSource = findIdxFilename(filenameSource, parentIndexSource);
            if (idxSource == FILE_NOT_FOUND)
            {
                notFound(argv);
            }
            else
            {
                goToFolder(pathTarget, &success, &parentIndexTarget);
                if (success == 1)
                {
                    // cek apakah source berupa file atau folder
                    idxSectorSource = getIdxFileSector(idxSource);
                    // cari index file target
                    idxTarget = findIdxFilename(filenameTarget, parentIndexTarget);

                    // kalau idxTarget FILE_NOT_FOUND, lakukan rename dan dipindahkan ke parentIndexTarget
                    if (idxTarget == FILE_NOT_FOUND)
                    {
                        if (compare2String(filenameTarget, "\0"))
                        { // kalau di move di root
                            rename(parentIndexTarget, idxSectorSource, filenameSource, idxSource);
                        }
                        else
                        {
                            rename(parentIndexTarget, idxSectorSource, filenameTarget, idxSource);
                        }
                        succeed(argv, argv + ARGS_LENGTH);
                    }
                    else
                    {
                        // cek apakah target berupa file atau folder
                        idxSectorTarget = getIdxFileSector(idxTarget);

                        // kalau berupa folder, dimasukkan
                        if (idxSectorTarget == 0xFF)
                        {
                            rename(idxTarget, idxSectorSource, filenameSource, idxSource);
                            succeed(argv, argv + ARGS_LENGTH);
                        }
                        // kalau sama2 file, di overwrite
                        else if (idxSectorSource != 0xFF)
                        {
                            deleteFile(filenameTarget, 0, parentIndexTarget);
                            rename(parentIndexTarget, idxSectorSource, filenameTarget, idxSource);
                            succeed(argv, argv + ARGS_LENGTH);
                        }
                        else
                        {
                            printString("mv: cannot overwrite non-directory '");
                            printString(argv + ARGS_LENGTH);
                            printString("' with directory '");
                            printString(argv);
                            printString("'");
                        }
                    }
                }
                else if (success == FILE_NOT_FOUND)
                {
                    notFound(argv + ARGS_LENGTH);
                }
                else
                {
                    notDir(argv + ARGS_LENGTH);
                }
            }
        }
        else if (success == FILE_NOT_FOUND)
        {
            notFound(argv);
        }
        else
        {
            notDir(argv);
        }
        printString("\n\r");
    }
    else
    {
        argTarget = argv + ((int)argc - 1) * ARGS_LENGTH;
        idxTarget = getIdxTarget(argTarget, parentIndexTarget);

        if (idxTarget == -1)
        {
            targetNotDir(argTarget);
            printString("\n\r");
        }
        else
        {
            for (i = 0; i < ((int)argc) - 1; ++i)
            {
                argSource = argv + i * ARGS_LENGTH;
                splitPath(argSource, pathSource, filenameSource);
                parentIndexSource = parentIndex;
                goToFolder(pathSource, &success, &parentIndexSource);
                if (success == 1)
                {
                    // cari index file source
                    idxSource = findIdxFilename(filenameSource, parentIndexSource);
                    if (idxSource != FILE_NOT_FOUND)
                    {
                        idxSectorSource = getIdxFileSector(idxSource);
                        rename(idxTarget, idxSectorSource, filenameSource, idxSource);
                        succeed(argSource, argTarget);
                    }
                    else
                    {
                        notFound(argSource);
                    }
                }
                else if (success == FILE_NOT_FOUND)
                {
                    notFound(argSource);
                }
                else
                {
                    notDir(argSource);
                }
                printString("\n\r");
            }
        }
    }
    while (1)
    {
    }
}

void succeed(char *source, char *dest)
{
    printString("mv: succeed to move '");
    printString(source);
    printString("' to '");
    printString(dest);
    printString("'");
}

void notFound(char *path)
{
    printString("mv: cannot stat '");
    printString(path);
    printString("' : No such file or directory");
}

void notDir(char *path)
{
    printString("mv: cannot stat '");
    printString(path);
    printString("' : Not a directory");
}

void targetNotDir(char *path)
{
    printString("mv: target '");
    printString(path);
    printString("' is not a directory");
}

void rename(char parentIndex, char idxSector, char *filename, int idxFile)
{
    char fileEntry[FILES_ENTRY_LENGTH];

    fileEntry[0] = parentIndex;
    fileEntry[1] = idxSector;
    copyString(filename, fileEntry + 2, 0);
    setFileEntry(fileEntry, idxFile); // rename source
}

int getIdxTarget(char *argTarget, char parentIndexTarget)
{
    char pathTarget[ARGS_LENGTH], filenameTarget[MAX_FILENAME];
    int success, idxTarget, idxSectorTarget;

    splitPath(argTarget, pathTarget, filenameTarget);
    goToFolder(pathTarget, &success, &parentIndexTarget);
    if (success == 1)
    {
        if (!compare2String(filenameTarget, "\0"))
        {
            idxTarget = findIdxFilename(filenameTarget, parentIndexTarget);
            if (idxTarget != FILE_NOT_FOUND)
            {
                // cek apakah target berupa file atau folder
                idxSectorTarget = getIdxFileSector(idxTarget);
                if (idxSectorTarget == 0xFF)
                {
                    return idxTarget;
                }
            }
        }
        else
        {
            return parentIndexTarget;
        }
    }
    return -1;
}