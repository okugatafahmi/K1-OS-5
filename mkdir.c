#include "lib/defines.h"
#include "lib/folder.h"

int main()
{
    char parentIndex, argc, argv[MAX_ARGS * ARGS_LENGTH];
    int i, success;

    getArgs(&parentIndex, &argc, argv);
    for (i = 0; i < (int)argc; ++i)
    {
        createFolder(argv + i * ARGS_LENGTH, &success, parentIndex);
        if (success == 1)
        {
            printString("mkdir: succeed to create directory '");
            printString(argv + i * ARGS_LENGTH);
            printString("'");
        }
        else
        {
            printString("mkdir: cannot create directory '");
            printString(argv + i * ARGS_LENGTH);
            if (success == FILE_HAS_EXIST)
            {
                printString("' : File exists");
            }
            else if (success == INSUFFICIENT_FILES)
            {
                printString("' : Insufficient directory");
            }
            else if (success == INVALID_FOLDER)
            {
                printString("' : No such file or directory");
            }
            else if (success == NOT_DIRECTORY)
            {
                printString("' : Not a directory");
            }
        }
        printString("\n\r");
    }
    executeProgram("shell",0x2000,0,0x1);
}
