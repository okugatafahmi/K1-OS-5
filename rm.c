#include "lib/defines.h"
#include "lib/utils.h"
#include "lib/teks.h"
#include "lib/folder.h"
#include "lib/file.h"

int main()
{
    char parentIndex, parentIndexTo, argc, argv[MAX_ARGS * ARGS_LENGTH];
    char pathTo[ARGS_LENGTH], filename[MAX_FILENAME], idxS;
    int i, idxTo, success;

    getArgs(&parentIndex, &argc, argv);
    for (i = 0; i < (int)argc; ++i)
    {
        splitPath(argv + i * ARGS_LENGTH, pathTo, filename);
        parentIndexTo = parentIndex;
        goToFolder(pathTo, &success, &parentIndexTo);
        if (success == 1)
        {
            idxTo = findIdxFilename(filename, parentIndexTo);
            if (idxTo == FILE_NOT_FOUND)
            {
                printString("rm: cannot remove '");
                printString(argv + i * ARGS_LENGTH);
                printString("' : No such file or directory");
            }
            else
            {
                idxS = getIdxFileSector(idxTo);
                if (idxS == 0xFF)
                {
                    deleteFolder(filename, 0, parentIndexTo);
                }
                else
                {
                    deleteFile(filename, 0, parentIndexTo);
                }
                printString("rm: succeed to remove '");
                printString(argv + i * ARGS_LENGTH);
                printString("'");
            }
        }
        else if (success == FILE_NOT_FOUND)
        {
            printString("rm: cannot remove '");
            printString(argv + i * ARGS_LENGTH);
            printString("' : No such file or directory");
        }
        else
        {
            printString("rm: cannot remove '");
            printString(argv + i * ARGS_LENGTH);
            printString("' : Not a directory");
        }
        printString("\n\r");
    }
    while (1)
    {
    }
}