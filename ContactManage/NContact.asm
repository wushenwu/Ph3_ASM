StackSeg segment stack
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
	gs_strNameHit	 db 0Dh, 0Ah, "Please Enter the Name:", 0Dh, 0Ah, "$"
	gs_strTelHit	 db 0Dh, 0Ah, "Please Enter the TelPhone:", 0Dh, 0Ah, "$"
	gs_strHead		 db 0Dh, 0Ah, "ID	Name		Tel", 0Dh, 0Ah, "$"
	gs_strIDHit		 db 0Dh, 0Ah, "Please Enter One ID to Del or Modify:", 0Dh, 0Ah, "$"
	gs_strCharTbl	 db "01234567890000000ABCDEF$"
	gs_strID		 db 5 dup(?)
	gs_strNewLine	 db 0Dh, 0Ah, "$"
					 
	;files to store contactinfo
	gs_strWholeInfoFile	 db "Info.bin", 00h
	
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
	gs_nIndex		dw 00h	;we should have use stack in the beginning......
DataSeg ends

CodeSeg segment
	ASSUME CS:CodeSeg, DS:DataSeg, ES:DataSeg, SS:StackSeg
	
ENTRY_POINT:
	call SEG_INIT
	call LOAD_TABLES
MAIN:
	mov dx, offset gs_strMainMenu
	call SHOW_MSG
	call GET_SELECT
	cmp al, 00h
	jz EXIT
	call DISPATCH
	jmp MAIN
	
;Add your ¡±subroutines¡° here
;----------------------Control---------------------
SEG_INIT:
	mov ax, DataSeg
	mov ds, ax
	mov es, ax
	
	mov ax, StackSeg
	mov ss, ax
	ret

;Dispatch according to user's choice
DISPATCH:
	cmp al, 01h
	jz INFO_SEARCH
	cmp al, 02h
	jz INFO_ADD_ENTRY
	cmp al, 03h
	jz INFO_DEL
	cmp al, 04h
	jz INFO_MODIFY
	cmp al, 05h
	jz INFO_LISTALL
DISPATCH_END:
	ret
	
;get length of string specified by ES:DI
;cx as the length
STRLEN:
	xor cx,cx
	cld
SCAN:
	inc cx
	mov al, "$"
	scasb
	jnz SCAN
	dec cx
	ret
	
;try to find substring SI within DI	
STRSTR:
	mov dx, di
	call STRLEN		
	mov ax, cx
	mov bx, ax
	
	mov di, si
	call STRLEN
	mov di, dx
	mov ax, bx
	mov bx, cx
	
	sub ax, bx		; if len(dst) < len(src)
	jb NOT_FOUND
	inc ax
	
	xor cx, cx
LOOP_DST:
	xor ah, ah
	cmp ax, cx
	jz LOOP_DST_END
	
	xor dx, dx
LOOP_SRC:
    mov bh, 0				;suppose found
	cmp bl, dl
	jz LOOP_SRC_END
	xor bp, bp				;compare [si+dx], [di+ cx + dx]
	add bp, di
	add bp, cx
	add bp, dx
	mov ah, byte ptr[bp]
	xor bp, bp
	add bp, si
	add bp, dx
	mov bh, byte ptr[bp]
	cmp ah, bh
	mov bh, -1				;suppose not found
	jnz LOOP_SRC_END
	inc dx
	jmp LOOP_SRC
	
LOOP_SRC_END:
    and bh, 0ffh
	jz FOUND
	inc cx
	jmp LOOP_DST
	
LOOP_DST_END:
	;bh = 0 found, -1 not found
	
NOT_FOUND:
	mov ax, -1
	ret
FOUND:
    mov ax, 0
	ret

;whether the gs_strName and (or) gs_strTel matches gs_strWholeInfo
;AX=0,if match; -1 not match
SEARCH_MATCH:
	mov si, offset gs_strName
	mov di, offset gs_strWholeInfo
	inc di
	xor ax, ax
	mov al, gs_nNameBufUsed
	test al, al
	jz SEARCH_TEL
	call STRSTR
	test ax, ax
	jnz  SEARCH_MATHCH_END		;Name not match
	
SEARCH_TEL:
	mov si, offset gs_strTel
	mov di, offset gs_strWholeInfo
	inc di
	xor ax, ax
	mov al, gs_nNameBufLen
	add di, ax
	xor ax, ax
	mov al, gs_nTelBufUsed
	test al, al
	jz SEARCH_MATHCH_END
	call STRSTR
	test ax, ax
	jnz  SEARCH_MATHCH_END		;Tel not match
								;now found, ax=0
SEARCH_MATHCH_END:
	ret

;Support Union and like search	
INFO_SEARCH:
	call GET_CONTACTINFO
	mov al, gs_nNameBufUsed
	or al, gs_nTelBufUsed		;only one can be empty
	jz INFO_SEARCH
	
	call SHOW_HEADER
	;now strstr
	xor si, si						;id 
	mov gs_nIndex, si
WALK_FILE_SEARCH:
	call SEEK						;try to seek to si * 31h
	
	mov bx, gs_hInfoFile			;read whole info
	mov cx, offset gs_strWholeInfoEnd - offset gs_strWholeInfo
	mov dx, offset gs_strWholeInfo
	call READ_FILE
	and ax, 0ffh					;ax=0, no data readed
	jz SEARCH_END
	
	and gs_strWholeInfo, 0ffh		;check flag
	jnz	SEARCH_NEXT					;deleted, skip to next
	
	call SEARCH_MATCH				;now try to match
	test ax, ax
	jz MATCHED					;found, show it
SEARCH_NEXT:
	mov si, gs_nIndex
	inc si	
	mov gs_nIndex, si
	jmp WALK_FILE_SEARCH	
MATCHED:
	mov si, gs_nIndex
	call SHOW_RECORD
	jmp SEARCH_NEXT
SEARCH_END:
	jmp DISPATCH_END

;Get name , telphone from user input and store into file
INFO_ADD_ENTRY:
	mov gs_nIndex, 0
	mov bp, 1				;1 ADD, no one can be empty;   0 Modify, can be empty
INFO_ADD:
	call GET_CONTACTINFO
	test bp, bp
	jz FIELD_CHECK_END		;if modify, no need to check
FIELD_CHECK:
	mov al, gs_nNameBufUsed
	and al, gs_nTelBufUsed		;no one can be empty
	jz INFO_ADD
FIELD_CHECK_END:

	mov si, gs_nIndex
WALK_FILE:
	call SEEK						;try to seek to si * 31h
	sbb cx, cx						;whether cf set
	jnz	WRITE_INFO					;no more to SEEK 
	
	mov bx, gs_hInfoFile			;
	mov cx, gs_strWholeInfoEnd - gs_strWholeInfo
	mov dx, offset gs_strWholeInfo
	call READ_FILE
	and ax, 0ffh					;ax=0, no data read
	jz WRITE_INFO
	
	and gs_strWholeInfo, 0ffh		;check flag
	jnz	WRITE_INFO					;deleted
	inc si	
	mov gs_nIndex, si
	jmp WALK_FILE					;if not, next
WRITE_INFO:
	call PRE_WRITE					;put ID, name, tel all together
	mov si, gs_nIndex				;need to seek
	call SEEK
	mov bx, gs_hInfoFile
	xor cx, cx
	mov cl, offset gs_strWholeInfoEnd - offset gs_strWholeInfo
	mov dx, offset gs_strWholeInfo
	call WRITE_FILE
	jmp DISPATCH_END

;delete the specified id
;ax=0 delete ok   ax=1 not exist	
INFO_DEL:	
	mov dx, offset gs_strIDHit
	call SHOW_MSG
	xor dx, dx
	xor cx, cx
	call GET_SELECT_ID				;dx get the ID
	test cx, cx
	jz DEL_ERROR
	
	mov si, dx
	mov gs_nIndex, si
	call SEEK						;try to seek to si * 31h
	
	mov bx, gs_hInfoFile			;read one byte
	mov cx, 1
	mov dx, offset gs_strWholeInfo
	call READ_FILE
	and ax, 0ffh					;ax=0, no data read
	jz DEL_ERROR
	
	and gs_strWholeInfo, 0ffh		;check flag
	jnz	DEL_ERROR					;deleted
	
	call SEEK						;try to seek to si * 31h;  Okay, can also use SEEK_CUR
	mov byte ptr[gs_strWholeInfo], 01h	;now modify the flag
	mov dx, offset gs_strWholeInfo
	mov cx, 1
	mov bx, gs_hInfoFile
	call WRITE_FILE
	mov ax, 0			;del ok
	jmp DEL_END
DEL_ERROR:	
	mov dx, offset gs_strIDError
	call SHOW_MSG
	mov ax, 1			;non exist
DEL_END:
	jmp DISPATCH_END

INFO_MODIFY:
	call INFO_DEL
	test ax, ax
	jnz INFO_MODIFY_END	;NON EXIST
	mov gs_nIndex, si	;si is the ID to be modified
	mov bp, 0			; 0 indicate modify
	call INFO_ADD
INFO_MODIFY_END:
	jmp DISPATCH_END

;walk through the file, and show those not deleted records	
INFO_LISTALL:
	call SHOW_HEADER
	
	xor si, si						;id 
	mov gs_nIndex, si
WALK_FILE_ALL:
	call SEEK						;try to seek to si * 31h
	sbb cx, cx						;whether cf set
	jnz	LIST_END					;no more to SEEK 
	
	mov bx, gs_hInfoFile			;read whole info
	mov cx, offset gs_strWholeInfoEnd - offset gs_strWholeInfo
	mov dx, offset gs_strWholeInfo
	call READ_FILE
	and ax, 0ffh					;ax=0, no data readed
	jz LIST_END
	
	and gs_strWholeInfo, 0ffh		;check flag
	jnz	WALK_NEXT					;deleted, skip to next
	
	call SHOW_RECORD				;if not, show this record
WALK_NEXT:
	mov si, gs_nIndex
	inc si		
	mov gs_nIndex, si
	jmp WALK_FILE_ALL				
LIST_END:
	jmp DISPATCH_END
	
;----------------------------------------------------------
LOAD_TABLES:
	mov dx, offset gs_strWholeInfoFile
	call OPEN_FILE
	mov [gs_hInfoFile], ax
	ret
	
;-------------------------User Input--------------------		
SHOW_MSG:
	mov ah, 09h
	int 21h
	ret
	
SHOW_BYTE:
	mov ah, 02h
	int 21h
	ret
	
SHOW_HEADER:
	mov dx, offset gs_strHead
	call SHOW_MSG
	ret	

;show ID  Name  Tel
;deal with ID with hex format	
SHOW_RECORD:
	;show ID in hex format			
	mov dx, si						;dx as id
	mov di, offset gs_strID
GET_ID:								;conver id into its hex string, saving to gs_strID
	cld
	mov ax, dx
	and ax, 000Fh
	stosb 							;save the last 4 bit into gs_strID
	mov cl, 4
	shr dx, cl
	jz SHOW_ID
	jmp GET_ID	
SHOW_ID:
	std
	mov si, di
	dec si
SHOW_NEXT:
	lodsb 							;now al get the char
	mov di, offset gs_strCharTbl
	add di, si
	inc di
	sub di, offset gs_strID			
	xor ah, ah
	add di, ax
	mov dl, byte ptr[di]
	call SHOW_BYTE
	cmp si, offset gs_strID - 1
	jnz SHOW_NEXT
	
	mov dl, 09h ;tab
	call SHOW_BYTE
	mov dx, offset gs_strWholeInfo + 01h		;show name
	call SHOW_MSG
	
	mov dl, 09h	;tab
	call SHOW_BYTE
	mov dl, 09
	call SHOW_BYTE
	xor cx, cx
	mov cl, gs_nNameBufLen
	mov dx, offset gs_strWholeInfo + 01h		;show telphone
	add dx, cx
	call SHOW_MSG
	
	mov dx, offset gs_strNewLine
	call SHOW_MSG
	ret
	
;get one byte input from user
GET_ONE_INPUT:
	mov ah, 01h
	int 21h
	ret
	
;Buffered Keyboard Input
GET_BUF_INPUT:
	mov ah, 0ah
	int 21h
	ret
	
;replace OD with '$'
CLEAR_OD:
	xor cx, cx
	mov cl, [si]
	mov si, cx
	mov byte ptr [bx+si], '$'
	ret
	
;get user's selection for ID, and convert it to hex value
;store into dx if cl=4
GET_SELECT_ID:
	mov ah, 01h
	int 21h
	cmp al, '0'
	jb SELECT_ID_END
	cmp al, 'F'
	ja SELECT_ID_END
	cmp al, '9'
	ja AF
	sub al, '0'
	jmp STR2HEX
AF:
	sub al, 'A'
	add al, 0Ah

STR2HEX:	
	mov cl, 4
	shl dx, cl
	xor ah, ah
	add dx, ax		; dx=dx*10H+al=dx<<4 + al	
	jmp GET_SELECT_ID
SELECT_ID_END:
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
	call SHOW_MSG
	jmp GET_SELECT
	
GET_CONTACTINFO:
	mov gs_nNameBufUsed, 0
	mov gs_nTelBufUsed, 0
	
	mov dx, offset gs_strNameHit
	call SHOW_MSG
	mov dx, offset gs_nNameBufLen
	call GET_BUF_INPUT
	
	mov si, offset gs_nNameBufUsed			; 0D--->$
	mov bx, offset gs_strName
	call CLEAR_OD
	
	mov dx, offset gs_strTelHit
	call SHOW_MSG
	mov dx, offset gs_nTelBufLen
	call GET_BUF_INPUT
		
	mov si, offset gs_nTelBufUsed
	mov bx, offset gs_strTel
	call CLEAR_OD
	ret
	
;---------------------File Operation--------------------
;copy block of bytes (cx) from si to di
MEMCPY:
	cld
	rep movsb
	ret
	
;put 01h, gs_strName, gs_strTel into gs_strWholeInfo, for one time writing
PRE_WRITE:
	mov gs_strWholeInfo, 00h
	mov si, offset gs_strName
	mov di, offset gs_strWholeInfo + 01h
	xor cx, cx
	mov cl, gs_nNameBufUsed
	test cl, cl
	jz COPY_TEL
	add cl, 1	;$
	call MEMCPY

COPY_TEL:
	mov si, offset gs_strTel
	xor cx, cx
	mov cl, gs_nNameBufLen
	mov di, offset gs_strWholeInfo + 01h
	add di, cx
	xor cx, cx
	mov cl, gs_nTelBufUsed
	test cl, cl
	jz COPY_END
	add cl, 1		;$
	call MEMCPY
COPY_END:
	ret
	
;loop throuth the file, checking flags, and
SEEK:
	mov bx, gs_hInfoFile			
	mov ax, offset gs_strWholeInfoEnd - offset gs_strWholeInfo
	mul si
	mov cx, dx
	mov dx, ax
	call SEEK_FILE	
	ret
	
OPEN_FILE:
	mov ah, 3dh
	mov al, 02h
	int 21h
	sbb cx, cx		;whether CF set
	jnz CREATE_FILE
	ret

CREATE_FILE:
	mov ah, 3ch
	mov cl, 40h		;archive
	int 21h
	ret

READ_FILE:
	mov ah, 3fh
	int 21h
	sbb cx, cx		;whether CF set, error occurs
	ret

WRITE_FILE:
	mov ah, 40h
	int 21h
	ret

SEEK_FILE:
	mov ah, 42h
	mov al, 00h
	int 21h
	ret
	
CLOSE_FILE:
	mov ah, 3eh
	int 21h
	ret
	
EXIT:
	mov bx, gs_hInfoFile
	call CLOSE_FILE
	mov ax, 4c00h
	int 21h
CodeSeg ends

end ENTRY_POINT