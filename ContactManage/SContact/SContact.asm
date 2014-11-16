StackSeg segment stack
	db 100h dup(0cch)
StackSeg ends

DataSeg segment
	;Main Menu
	gs_strMainMenu   db 0Dh, 0Ah
					 db "1. Search	-- Search with Name and (or) TelPhone", 0Dh, 0Ah
					 db "2. Add		-- Store the Name and TelPhone",		0Dh, 0Ah
					 db "3. Del		-- Delete A Specific Record, ID needed",0Dh, 0Ah
					 db "4. Modify	-- Modify the Specific Record with New Info", 0Dh, 0Ah
					 db "5. All		-- List All the Info", 0Dh, 0Ah
					 db "0. Exit	-- See You Next Time", 0Dh, 0Ah
					 db "Please Select the Number You Want to Deal With:", 0Dh, 0Ah, "$"
					 
	;hit info
	gs_strIllHit     db 0Dh, 0Ah, "Illegal Input! Please Enter Again!", 0Dh, 0Ah, "$"
	gs_strIDError	 db 0Dh, 0Ah, "This ID Not EXIST!!!", 0Dh, 0Ah, "$"
	gs_strLoadError  db 0Dh, 0Ah, "Failed to Open and Create File!! Exist Now!!", 0Dh, 0Ah, "$"
	gs_strNameHit	 db 0Dh, 0Ah, "Please Enter the Name:", 0Dh, 0Ah, "$"
	gs_strTelHit	 db 0Dh, 0Ah, "Please Enter the TelPhone:", 0Dh, 0Ah, "$"
	gs_strHead		 db 0Dh, 0Ah, "ID	Name		Tel", 0Dh, 0Ah, "$"
	gs_strIDHit		 db 0Dh, 0Ah, "Please Enter One ID to Del or Modify:", 0Dh, 0Ah, "$"
	gs_strCharTbl	 db "01234567890000000ABCDEF$"
	gs_strID		 db 5 dup(?)
	gs_strNewLine	 db 0Dh, 0Ah, "$"
					 
	;files to store contactinfo
	gs_strInfoFile	 db "Info.bin", 00h
	
	;filehandle
	gs_hInfoFile	dw	?
	gs_hAddFile		dw	?
	gs_hDelFile		dw	?
	
	;tmp info from user input
	gs_nNameBufLen 	db 20h			; used for int 21h, 0Ah, get name input from user
	gs_nNameBufUsed db ?			;
	gs_strName   	db 20h dup(?)	; to store name input from user or read from file
	
	gs_nTelBufLen 	db 10h			; just as gs_nNameBufLen, for Tel
	gs_nTelBufUsed  db ?			;
	gs_strTel  		db 10h dup(?)   ; just as gs_strName, fro Tel		
	
	;whole block read from file or to write into file
	;1byte deleted flag + 20hbyte Name + 10hbyte Tel
	gs_strWholeInfo	db 31h	dup(01h)
	gs_strWholeInfoEnd db ?
	
	;just a trick
	gs_bModify		db 00h  ; 0 Add  1 Modify
	gs_nIndex		dw 00h	; when gs_bModify is 1, gs_nIndex indicate the record ID
	
DataSeg ends

CodeSeg segment
	ASSUME CS:CodeSeg, DS:DataSeg, ES:DataSeg, SS:StackSeg
	
ENTRY_POINT:
	push bp			;stack setup
	mov bp, sp
	
	call SEG_INIT
	call LOAD_TABLES
	test ax, ax		;0 success, -1 failed
	jz MAIN
	
	mov dx, offset gs_strLoadError
	call SHOW_INFO
	jmp EXIT
MAIN:
	mov dx, offset gs_strMainMenu
	call SHOW_INFO
	call GET_SELECT
	test al, al
	jz EXIT
	call DISPATCH
	jmp MAIN
	
;Add your ¡±subroutines¡° here
;----------------------Control---------------------
SEG_INIT:
	push bp
	mov bp, sp
	
	mov ax, DataSeg
	mov ds, ax
	mov es, ax
	
	mov ax, StackSeg
	mov ss, ax
	
	mov sp, bp
	pop bp
	ret

DISPATCH_TABLE:
	dw (INFO_SEARCH)
	dw (INFO_ADD)
	dw (INFO_DEL)
	dw (INFO_MODIFY)
	
;Dispatch according to user's choice
;We just build the DISPATCH_TABLE
DISPATCH:
	sub al, 01h
	xor ah, ah
	shl al, 1
	mov si, ax
	mov bx, DISPATCH_TABLE
	mov bx, cs:[bx+si]
	call bx
	ret
	
INFO_SEARCH:
	ret

;Get name , telphone from user input, and store these info into proper position
;Params : gs_bModify = 1  modifying data, should use gs_nIndex as the record ID, 
;						and no need to loop through file to find proper position
;		  gs_bModify = 0  just add data, should loop through file to find proper position
INFO_ADD:
	push bp
	mov bp, sp
	
	call GET_CONTACTINFO
	
	mov al, gs_nNameBufUsed
	and al, gs_nTelBufUsed		
	jz INFO_ADD				;No one can be empty
	
	mov al, gs_bModify
	test al, al
	jz  LOOP_THROUGH		;simple add, need to loop through
	call LOCATE				;modifying case, locate it directly
	jmp WRITE_INFO	
	
	mov gs_nIndex, 0
LOOP_THROUGH:
	call LOCATE
	test bx, bx
	jnz	XXX	;locate failed
	
	
WRITE_INFO:
	
	mov bp, sp
	pop bp	
	ret
	
INFO_DEL:	
	ret

INFO_MODIFY:
	ret
	
INFO_LISTALL:
	ret

;Function: loop through file, and read record into gs_strWholeInfo, check delete flag, 
;			find the proper position to write data
;Params  : gs_bModify=1  modifying, gs_nIndex indicate the record ID, no need to loop through
;		   gs_bModify=0  simple add, need to loop through 
;Return  : gs_bModify=0 gs_nIndex as record ID, this record already deleted, can be overwritten
;			gs_strWholeInfo always holds the record readed last time	
FIND_POSITION:
	push bp
	mov bp, sp
	
	mov al, gs_bModify
	test al, al
	jz  LOOP_THROUGH		;simple add, need to loop through
	call LOCATE				;modifying case, locate it directly
	jmp xxxx	
	
	mov gs_nIndex, 0
LOOP_THROUGH:
	call LOCATE
	test bx, bx
	jnz	FIND_POSITION_END	;locate failed
	
	
FIND_POSITION_END:
		
	
	mov sp, bp
	pop bp
	ret
	
;-------------------------Tables Operation--------------

;Function: try to open file, if not exist, create the file
;Params  : None
;Return  : ax=0 success
;		   ax=-1 failed
LOAD_TABLES:	
	push bp
	mov bp, sp
	
	;firstly, try to open
	mov dx, offset gs_strInfoFile
	push dx
	call OPEN_FILE
	pop dx					
	test dx, dx				;0 success, -1 failed
	jz LOAD_TABLES_END
	
	;open failed, now try to create the file
	mov dx, offset gs_strInfoFile
	push dx
	call CREATE_FILE
	pop dx					
	test dx, dx				;0 success, -1 failed
	jz LOAD_TABLES_END
	
	mov ax, -1
	jmp LOAD_TABLES_CLEAN_UP

LOAD_TABLES_END:
	mov [gs_hInfoFile], ax
	mov ax, 0

LOAD_TABLES_CLEAN_UP:	
	mov sp, bp
	pop bp
	ret	

;-------------------------User Input--------------------	
;Function: Show the info specified by DS:DX	
SHOW_INFO:
	mov ah, 09h
	int 21h
	ret
	
;Function: get one byte input from user
;Params  ; None
;Return	 : AL
GET_ONE_INPUT:
	mov ah, 01h
	int 21h
	ret
	
;Buffered Keyboard Input
GET_BUF_INPUT:
	mov ah, 0ah
	int 21h
	ret
	
;Function: replace OD (DS:[bx+si]) with '$'
;Params  : bx points to the Buf
;          si is the index of 0D within the Buf
;Return  : None  
CLEAR_OD:
	xor cx, cx
	mov cl, [si]
	mov si, cx
	mov byte ptr [bx+si], '$'
	ret
	
;get user's select for Main Menu
GET_SELECT:
	call GET_ONE_INPUT
	and al, 0CFh
	cmp al, 05h
	ja SELECT_AGAIN
	ret
SELECT_AGAIN:
	mov dx, offset gs_strIllHit
	call SHOW_INFO
	jmp GET_SELECT
	
GET_CONTACTINFO:
	mov dx, offset gs_strNameHit
	call SHOW_INFO
	mov dx, offset gs_nNameBufLen
	call GET_BUF_INPUT
	mov si, offset gs_nNameBufUsed
	mov bx, offset gs_strName
	call CLEAR_OD
	
	mov dx, offset gs_strTelHit
	call SHOW_INFO
	mov dx, offset gs_nTelBufLen
	call GET_BUF_INPUT
	mov si, offset gs_nTelBufUsed
	mov bx, offset gs_strTel
	call CLEAR_OD
	
;---------------------File Operation------------------------
;Function: try to open file specified by param1
;		   and set param1 to indicate the status
;Param: param1 points to file name
;Return: param1 0  success  
;               -1  failed
;		 ax	    FileHandle when param1 set to 0
OPEN_FILE:
	push bp
	mov bp, sp
	
	mov dx, [bp+4]
	mov ah, 3dh
	mov al, 02h
	int 21h
	
	sbb dx, dx	;check and set
	mov [bp+4], dx
	
	mov sp, bp
	pop bp
	ret

;Function: try to create file (as archive) named by param1
;		   and set param1 to indicate the status
;Param: param1 points to file name
;Return: param1 0  success  
;               -1  failed
;		 ax	    FileHandle when param1 set to 0
CREATE_FILE:
	push bp
	mov bp, sp
	
	mov dx, [bp+4]
	mov ah, 3ch
	mov cl, 40h		;archive
	int 21h
	
	sbb dx, dx	;check and set
	mov [bp+4], dx
	
	mov sp, bp
	pop bp
	ret

READ_FILE:
	mov ah, 3fh
	int 21h
	ret

WRITE_FILE:
	mov ah, 40h
	int 21h
	ret

;Function: locate the record indicated by gs_nIndex, it's position is 31h * gs_nIndex
;Params  : gs_nIndex as the record ID
;Return  : bx=0 success 
;          bx=-1 failed
LOCATE:
	mov bx, gs_hInfoFile			
	mov ax, offset gs_strWholeInfoEnd - offset gs_strWholeInfo
	mov cx, gs_nIndex
	mul cx
	mov cx, dx
	mov dx, ax
	call SEEK_FILE		;bx=0 success, bx=-1 failed
	ret
	
;Function: seek to position indicated by CX:DX
;Params: AL as mode
;		 BX as filehandle
;	     CX:DX,  CX as high order word of number of bytes to move,
;				 DX as low order word of number of bytes to move
;Return: BX = 0  DX:AX  indicate the new position
;		 BX = -1 error occurs
SEEK_FILE:
	mov ah, 42h
	int 21h
	
	sbb bx, bx	; check and set
	
	ret
	
EXIT:
	mov sp, bp		;stack cleanup
	pop bp
	mov ax, 4c00h
	int 21h
CodeSeg ends

end ENTRY_POINT