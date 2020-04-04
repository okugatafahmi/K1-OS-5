#include "video.h"

void clrscr(){
    interrupt(0x10,2,0,0,0);
}

void clearLine(){
    char row,col=0;
    while (col++<=MAX_COL){
        interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
    }
}

void getPos(char *row, char *col){
    int ax=0x3<<8,bx=0,cx=0,dx=0;
    interruptEdit(0x10,&ax,&bx,&cx,&dx);
    *row = (char)(dx>>8); *col = (char) dx;
}

void setPos(char row, char col){
    interrupt(0x10,0x2<<8,0,0,row<<8|col);
}

void scrollUp(){
    char row,col;
    interrupt(0x10,0x6<<8|LINE_SCROLL,0x7<<8,MIN_ROW<<8|MIN_COL,MAX_ROW<<8|MAX_COL);
    getPos(&row, &col);
    setPos(row-LINE_SCROLL,col);
}

void scrollDown(){
    char row,col;
    interrupt(0x10,0x7<<8|LINE_SCROLL,0x7<<8,MIN_ROW<<8|MIN_COL,MAX_ROW<<8|MAX_COL);
    getPos(&row, &col);
    setPos(row+LINE_SCROLL,col);
}

void printAtRow(char *string, char row){
    char rowNow,colNow;
    getPos(&rowNow,&colNow);
    setPos(row,0);
    interrupt(0x21, 0x0,string,0,0);
    setPos(rowNow,colNow);
}