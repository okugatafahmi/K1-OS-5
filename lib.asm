;lib.asm
;Michael Black, 2007
;Modified by Asisten Sister, 2020

;lib.asm contains assembly functions that you can use in the shell

global _interrupt
global _interruptEdit

;int interrupt (int number, int AX, int BX, int CX, int DX)
_interrupt:
	push bp
	mov bp,sp
	mov ax,[bp+4]	;get the interrupt number in AL
	push ds		;use self-modifying code to call the right interrupt
	mov bx,cs
	mov ds,bx
	mov si,intr
	mov [si+1],al	;change the 00 below to the contents of AL
	pop ds
	mov ax,[bp+6]	;get the other parameters AX, BX, CX, and DX
	mov bx,[bp+8]
	mov cx,[bp+10]
	mov dx,[bp+12]

intr:	int 0x00	;call the interrupt (00 will be changed above)

	mov ah,0	;we only want AL returned
	pop bp
	ret

;int interruptEdit (int number, int AX, int BX, int CX, int DX)
_interruptEdit:
	push bp
	mov bp,sp
	mov ax,[bp+4]	;get the interrupt number in AL
	push ds		;use self-modifying code to call the right interrupt
	mov bx,cs
	mov ds,bx
	mov si,intre
	mov [si+1],al	;change the 00 below to the contents of AL
	pop ds
	mov bx, [bp+6]
	mov ax,[bx]	;get the other parameters AX, BX, CX, and DX
	mov bx,[bp+10]
	mov cx,[bx]
	mov bx, [bp+12]
	mov dx,[bx]
	mov bx, [bp+8]
	mov bx, [bx]

intre:	int 0x00	;call the interrupt (00 will be changed above)
	
	push bx
	push ax
	push bx
	mov bx, [bp+6]		; set AX
	mov [bx], ax
	mov bx, [bp+10]		; set CX
	mov [bx], cx
	mov bx, [bp+12]		; set DX
	mov [bx], dx
	pop ax
	mov bx, [bp+8]		; set BX
	mov [bx], ax
	pop ax
	mov ah,0	;we only want AL returned
	pop bx
	pop bp
	ret