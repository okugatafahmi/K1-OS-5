#include "lib/defines.h"
#include "lib/utils.h"
#include "lib/file.h"
#include "lib/teks.h"

int main()
{
    char buffer[SECTOR_SIZE * MAX_FILESECTOR], argv[MAX_ARGS * ARGS_LENGTH];
    char parentIndex, argc;
    int result;
    int i;

    // baca argumen, copy argumen ke fileName
    getArgs(&parentIndex, &argc, argv);

    for (i = 0; i < (int)argc; ++i)
    {
        // read file terus print
        readFile(buffer, argv + i * ARGS_LENGTH, &result, parentIndex);
        if (result == 1)
        {
            printString(buffer);
        }
        else if (result == FILE_IS_DIRECTORY)
        {
            printString("cat: ");
            printString(argv + i * ARGS_LENGTH);
            printString(": Is a directory");
        }
        else
        {
            printString("cat: ");
            printString(argv + i * ARGS_LENGTH);
            printString(": No such file or directory");
        }
        printString("\n\r");
    }
    while (1)
    {
    }
}
