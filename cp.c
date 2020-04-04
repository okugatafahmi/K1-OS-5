#include "lib/defines.h"
#include "lib/utils.h"
#include "lib/teks.h"
#include "lib/folder.h"
#include "lib/file.h"

void succeed(char *source, char *dest);
void notSucceed(char *source, char *dest);
void notFound(char *path);
void notDir(char *path);
void ketCopy(int success, char *source, char *dest);
void targetNotDir(char *path);
int getIdxTarget(char *argTarget, char parentIndexTarget);

int main(){
    char parentIndex, parentIndexSource, parentIndexTarget, argc, argv[MAX_ARGS*ARGS_LENGTH];
    char pathSource[ARGS_LENGTH], filenameSource[MAX_FILENAME];
    char pathTarget[ARGS_LENGTH], filenameTarget[MAX_FILENAME];
    char buffer[MAX_FILESECTOR*SECTOR_SIZE];
    char fileEntry[FILES_ENTRY_LENGTH];
    char idxSectorSource, idxSectorTarget, *argSource, *argTarget;
    int i, idxSource, idxTarget, success, sectors;

    getArgs(&parentIndex, &argc, argv);
    parentIndexSource = parentIndex;    parentIndexTarget = parentIndex;
    if (argc<2){
        printString("cp: missing destination file operand after '");
        printString(argv);
        printString("'\n\r");
    }
    else if (argc==2){
        splitPath(argv,pathSource,filenameSource);
        splitPath(argv+ARGS_LENGTH, pathTarget, filenameTarget);
        // cari path source filenya
        goToFolder(pathSource, &success, &parentIndexSource);
        if (success == 1){
            // cari index file source
            idxSource = findIdxFilename(filenameSource, parentIndexSource);
            if (idxSource == FILE_NOT_FOUND){
                notFound(argv);
            }
            else{
                goToFolder(pathTarget, &success, &parentIndexTarget);
                if (success == 1){
                    // cek apakah source berupa file atau folder
                    idxSectorSource = getIdxFileSector(idxSource);
                    // kalau dia file baca dulu isinya
                    if (idxSectorSource!=0xFF){
                        readFile(buffer,filenameSource,0,parentIndexSource);
                        sectors = countSector(buffer);
                    }
                    // cari index file target
                    idxTarget = findIdxFilename(filenameTarget, parentIndexTarget);
                    
                    // kalau idxTarget FILE_NOT_FOUND, copy dengan nama target dan ditaruh di parentIndexTarget
                    if (idxTarget == FILE_NOT_FOUND){
                        if (compare2String(filenameTarget, "\0") ){ // kalau di move di root
                            if (idxSectorSource != 0xFF) writeFile(buffer,filenameSource,&sectors,parentIndexTarget);
                            else {
                                copyFolder(filenameSource,filenameSource,&success,parentIndexSource,parentIndexTarget);
                            }
                        }
                        else{
                            if (idxSectorSource != 0xFF) writeFile(buffer,filenameTarget,&sectors,parentIndexTarget);
                            else {
                                copyFolder(filenameSource,filenameTarget,&success,parentIndexSource,parentIndexTarget);
                            }
                        }
                        
                        if (success == 1){
                            succeed(argv,argv+ARGS_LENGTH);
                        }
                        else{
                            notSucceed(argv,argv+ARGS_LENGTH);
                        }
                    }
                    else{
                        // cek apakah target berupa file atau folder
                        idxSectorTarget = getIdxFileSector(idxTarget);

                        // kalau berupa folder, dimasukkan
                        if (idxSectorTarget == 0xFF){
                            if (idxSectorSource != 0xFF) writeFile(buffer,filenameTarget,&sectors,(char) idxTarget);
                            else {
                                copyFolder(filenameSource,filenameSource,&success,parentIndexSource,parentIndexTarget);
                            }
                            ketCopy(success,argv,argv+ARGS_LENGTH);
                        }
                        // kalau sama2 file, di overwrite
                        else if (idxSectorSource != 0xFF){
                            deleteFile(filenameTarget, 0, parentIndexTarget);
                            writeFile(buffer, filenameTarget, &sectors, parentIndex);
                            ketCopy(success,argv,argv+ARGS_LENGTH);
                        }
                        else{
                            printString("cp: cannot overwrite non-directory '");
                            printString(argv+ARGS_LENGTH);
                            printString("' with directory '");
                            printString(argv);
                            printString("'");
                        }
                    }
                }
                else if (success==FILE_NOT_FOUND){
                    notFound(argv+ARGS_LENGTH);
                }
                else{
                    notDir(argv+ARGS_LENGTH);
                }
            }
        }
        else if (success==FILE_NOT_FOUND){
            notFound(argv);
        }
        else{
            notDir(argv);
        }
        printString("\n\r");
    }
    else{
        argTarget = argv+((int)argc-1)*ARGS_LENGTH;
        idxTarget = getIdxTarget(argTarget, parentIndexTarget);
        
        if (idxTarget==-1){
            targetNotDir(argTarget);
            printString("\n\r");
        }
        else{
            for (i=0; i<((int)argc)-1; ++i){
                argSource = argv+i*ARGS_LENGTH;
                splitPath(argSource,pathSource,filenameSource);
                parentIndexSource = parentIndex;
                goToFolder(pathSource, &success, &parentIndexSource);
                if (success==1){
                    // cari index file source
                    idxSource = findIdxFilename(filenameSource, parentIndexSource);
                    if (idxSource != FILE_NOT_FOUND){
                        idxSectorSource = getIdxFileSector(idxSource);
                        // rename(idxTarget,idxSectorSource,filenameSource,idxSource);
                        succeed(argSource,argTarget);
                    }
                    else{
                        notFound(argSource);
                    }
                }
                else if (success==FILE_NOT_FOUND){
                    notFound(argSource);
                }
                else{
                    notDir(argSource);
                }
                printString("\n\r");
            }
        }
    }
    while(1){}
}

void succeed(char *source, char *dest){
    printString("cp: succeed to copy '");
    printString(source);
    printString("' to '");
    printString(dest);
    printString("'");
}

void notSucceed(char *source, char *dest){
    printString("cp: not succeed to copy '");
    printString(source);
    printString("' to '");
    printString(dest);
    printString("'");
}

void notFound(char *path){
    printString("cp: cannot stat '");
    printString(path);
    printString("' : No such file or directory");
}

void notDir(char *path){
    printString("cp: cannot stat '");
    printString(path);
    printString("' : Not a directory");
}

void targetNotDir(char *path){
    printString("cp: target '");
    printString(path);
    printString("' is not a directory");
}

void ketCopy(int success, char *source, char *dest){
    if (success == 1){
        succeed(source,dest);
    }
    else if (success == INSUFFICIENT_FILES){
            printString("cp: insufficient files");
    }
    else if (success == INSUFFICIENT_SECTORS){
        printString("cp: insufficient sector of file");
    }
    else{
        notSucceed(source,dest);
    }
}

int getIdxTarget(char *argTarget, char parentIndexTarget){
    char pathTarget[ARGS_LENGTH], filenameTarget[MAX_FILENAME];
    int success, idxTarget, idxSectorTarget;

    splitPath(argTarget, pathTarget, filenameTarget);
    goToFolder(pathTarget, &success, &parentIndexTarget);
    if (success == 1){
        if (!compare2String(filenameTarget,"\0")){
            idxTarget = findIdxFilename(filenameTarget, parentIndexTarget);
            if (idxTarget!=FILE_NOT_FOUND){
                // cek apakah target berupa file atau folder
                idxSectorTarget = getIdxFileSector(idxTarget);
                if (idxSectorTarget==0xFF){
                    return idxTarget;
                }
            }
        }
        else{
            return parentIndexTarget;
        }
    }
    return -1;
}