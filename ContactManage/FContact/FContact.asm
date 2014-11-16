include FContact_inc.asm

StackSeg segment stack
	db 256 dup(0cch)
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
	invoke SegInit
	invoke LoadTables, offset gs_strWholeInfoFile
	test ax, ax
	jnz EXIT
MAIN:
	invoke ShowMsg, offset gs_strMainMenu
	invoke GetSelect
	cmp al, 00h
	jz EXIT
	invoke Dispatch, al
	jmp MAIN
	
;Add your ¡±subroutines¡° here
;----------------------Control---------------------
;Function: init for ds, es,ss
;Params  : None
;Return  : None
SegInit proc stdcall
	mov ax, DataSeg
	mov ds, ax
	mov es, ax
	
	mov ax, StackSeg
	mov ss, ax
	ret
SegInit endp

DISPATCH_TABLE:
	dw (INFO_SEARCH)
	dw (INFO_ADD_ENTRY)
	dw (INFO_DEL)
	dw (INFO_MODIFY)
	dw (INFO_LISTALL)

;Function: Dispatch according to user's choice
;Params  : byte nChoice as the user's choice
;Return  : None
Dispatch proc stdcall uses bx si nChoice:byte
	mov al, nChoice
	sub al, 01h
	xor ah, ah
	shl al, 1
	mov si, ax
	mov bx, DISPATCH_TABLE
	mov bx, cs:[bx+si]
	call bx
	ret
Dispatch endp
	
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
	;call GET_CONTACTINFO
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
	ret

;Function: Get name , telphone from user input and store into file
INFO_ADD_ENTRY:
	mov gs_nIndex, 0
	mov bp, 1				;1 ADD, no one can be empty;   0 Modify, can be empty

;Function: get name, telphone from user input and store into file
;Params  : bModify=0 a simple add , need to loop through file 
;								to find proper position to write data
;					 nIndex is non-sence
;		   bModify=1 modifying data, nIndex indicate the record index to be overwritten
InfoAdd proc stdcall uses bx bModify:byte, nIndex:word
	local @bSucceed:word
	local @RecordInput:tagRecordInput
	local @RecordRead :tagRecordRead
	mov @RecordInput.m_nNameBufLen, 20h
	mov @RecordInput.m_nTelBufLen, 20h
	
	lea bx, @RecordInput
	invoke GetContactInfo, bx
	mov al, bModify
	test al, al
	jnz FIELD_CHECK_END		;if modifying, no need to check
	mov nIndex, 0			;if a simple add, nIndex used to loop through file
FIELD_CHECK:
	mov al, @RecordInput.m_nNameBufUsed
	and al, @RecordInput.m_nTelBufUsed		;no one can be empty
	invoke InfoAdd, bModify, nIndex
FIELD_CHECK_END:
	
	;now need to loop through or directly seek to the nIndex record
	mov cx, nIndex
WALK_FILE:
	invoke LocateRecord, cx			;
	test ax, ax
	jnz	WRITE_INFO					;no more to SEEK 
	
	lea dx, @RecordRead
	lea ax, @bSucceed
	invoke ReadFile, gs_hInfoFile, sizeof tagRecordRead, dx, ax
	mov dx, @bSucceed
	test dx, dx						;check success
	jnz WRITE_INFO
	
	mov al, @RecordRead.m_bDeleted
	test al, al						;check flag
	jnz	WRITE_INFO					;deleted
	inc cx	
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
	ret
InfoAdd endp

;delete the specified id
;ax=0 delete ok   ax=1 not exist	
INFO_DEL:	
	invoke ShowMsg, offset gs_strIDHit
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
	invoke ShowMsg, offset gs_strIDError
	mov ax, 1			;non exist
DEL_END:
	ret

INFO_MODIFY:
	call INFO_DEL
	test ax, ax
	jnz INFO_MODIFY_END	;NON EXIST
	mov gs_nIndex, si	;si is the ID to be modified
	mov bp, 0			; 0 indicate modify
;	call INFO_ADD
INFO_MODIFY_END:
	ret

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
	ret
	
;----------------------------------------------------------
;Function: Try to open the file; create it if not exist
;Params  : pointer to file name
;Return  : ax=0 if success; otherwise ax=-1
LoadTables proc stdcall uses bx pstrInfoFile:word
	local @bSucceed:word
	mov @bSucceed, 0
	
	lea bx, @bSucceed
	invoke OpenFile, pstrInfoFile, bx	;try to open file
	mov bx, @bSucceed
	test bx, bx
	jnz LOADTABLES_OPEN_FAILED
	mov gs_hInfoFile, ax			;save FileHandle if succeed
	mov ax, 0
	ret	
LOADTABLES_OPEN_FAILED:				;now try to create file
	lea bx, @bSucceed
	invoke CreateFile, pstrInfoFile, bx	;try to create file
	mov bx, @bSucceed
	test bx, bx
	jnz LOADTABLES_END
	mov gs_hInfoFile, ax			;save FileHandle if succeed	
	mov ax, 0
	ret
LOADTABLES_END:
	mov ax, -1
	ret
LoadTables endp
	
;-------------------------User Input--------------------
;Function: show info pointed by pstrInfo
;Params  : word ptr pstrInfo points to the info to be shown
;Return  : None		
ShowMsg proc stdcall uses dx pstrInfo:word
    mov dx, pstrInfo
	mov ah, 09h
	int 21h
	ret
ShowMsg endp
	
SHOW_BYTE:
	mov ah, 02h
	int 21h
	ret
	
SHOW_HEADER:
	invoke ShowMsg, offset gs_strHead
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
	invoke ShowMsg, offset gs_strWholeInfo + 01h
	
	mov dl, 09h	;tab
	call SHOW_BYTE
	mov dl, 09
	call SHOW_BYTE
	xor cx, cx
	mov cl, gs_nNameBufLen
	mov dx, offset gs_strWholeInfo + 01h		;show telphone
	add dx, cx
	invoke ShowMsg, dx
	
	invoke ShowMsg, offset gs_strNewLine
	ret
	
;Function: get one byte input from user
;Params  : None
;Return  : al as the input
GetOneInput proc stdcall
	mov ah, 01h
	int 21h
	ret
GetOneInput endp
	
;Function: Buffered Keyboard Input
;Params  : word ptr pBuff points to the buf to receive data
;Return  : None
;Alert   : pBuff is the ptr for Stack
GetBufInput proc stdcall uses ds pBuff:word
    mov ax, ss
	mov ds, ax		;use for ds:dx
	mov dx, [pBuff]
	mov ah, 0ah
	int 21h
	ret
GetBufInput endp
	
;Function: replace OD with '$' to end the buf
;Params  : word ptr pBuff points to the buf, 
;		   nLen is the OD's index
;Return  : None
ClearOD proc stdcall uses bx si pBuff:word, nLen:byte
	mov bx, [pBuff]
	xor cx, cx
	mov cl, nLen
	mov si, cx
	mov byte ptr [bx+si], '$'
	ret
ClearOD endp
	
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
	
;Function: get user's selection for Main Menu
;Params  : None
;Return  : al as the choice
GetSelect proc stdcall
	invoke GetOneInput
	and al, 0CFh
	cmp al, 05h
	ja SELECT_AGAIN
	ret
SELECT_AGAIN:
	invoke ShowMsg, offset gs_strIllHit
	invoke GetSelect
GetSelect endp

;Function: Get info about name, tel, and store into buf pointed by pRecordInput,
;		   used for Add, Search, and Modify
;Params  : word ptr pRecordInput is the buf to receive the input
;Return  : None
GetContactInfo proc stdcall uses bx pRecordInput:word
	mov bx, [pRecordInput]
	assume bx: ptr tagRecordInput
	
	invoke ShowMsg, offset gs_strNameHit
	invoke GetBufInput, bx
	invoke ClearOD, bx, [bx].m_nNameBufUsed
	
	invoke ShowMsg, offset gs_strTelHit
	invoke GetBufInput, bx
	
	mov al, [bx].m_nTelBufUsed
	add bl, [bx].m_nNameBufLen
	adc bh, 0
	invoke ClearOD, bx, al
	ret
GetContactInfo endp
	
;---------------------File Operation--------------------
;Function: copy block of bytes (cx) from si to di
MEMCPY:
	cld
	rep movsb
	ret
	
;Function: use pSrcBuff (tagRecordInput) to prepare pDstBuf (tagRecordRecord)
;Param   : word ptr pSrcBuff is a ptr for tagRecordInput
;		   word ptr pDstBuff is a ptr for tagRecordRead
;Return  : None
PreWrite proc stdcall uses si di pSrcBuff:word, pDstBuff:word
	mov si, [pSrcBuff]
	assume si: ptr tagRecordInput
	mov di, [pDstBuff]
	assume di: ptr tagRecordRead
	
	mov [di].m_bDeleted, 00h
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
	
;Function: seek to the nIndex record
;Params  : nIndex indicate which record
;Return  : ax=0 success  -1 failed
LocateRecord proc stdcall nIndex:word
	mov ax, sizeof tagRecordRead
	mul nIndex
	mov cx, dx
	mov dx, ax
	invoke SeekFile, gs_hInfoFile, cx, dx, 00h
	ret

;Function: try to open the file specified by pstrFileName
;Params  : word ptr pstrFileName  points to filename
;		   word ptr pbSucceed  used to receive the status
;Return  : if success, [pbSucceed] set to 0, and ax as filehandle
;             failed, [pbSucceed] set to -1
OpenFile proc stdcall uses dx bx pstrFileName:word, pbSucceed:word
	mov dx, pstrFileName
	mov ah, 3dh
	mov al, 02h
	int 21h
	
	sbb bx, bx				;whether CF set
	mov bx, [pbSucceed]		;set status
	jnz OPENFILE_FAILED
	mov ss:word ptr[bx], 0
	ret
OPENFILE_FAILED:
	mov ss:word ptr[bx], -1
	ret
OpenFile endp

;Function: try to create file specified by pstrFileName
;Params  : word ptr pstrFileName points to filename
;			word ptr pbSucceed  used to receive the status
;Return	 : if success, [pbSucceed] set to 0, and ax as filehandle
;             failed, [pbSucceed] set to -1
CreateFile proc stdcall uses dx bx pstrFileName:word, pbSucceed:word
	mov dx, pstrFileName
	mov ah, 3ch
	mov cl, 40h		;archive
	int 21h
	
	sbb bx, bx				;whether CF set
	mov bx, [pbSucceed]		;set status
	jnz CREATEFILE_FAILED
	mov ss:word ptr[bx], 0
CREATEFILE_FAILED:
	mov ss:word ptr[bx], -1
	ret
CreateFile endp

;Function: read the specified number of bytes into buf from the file
;Params  : hFile as file handle
;		   nCount number of bytes to read
;		   pBuff  buf to receive the data
;		   pbSucceed used to receive status
;Return  : [pbSucceed]=0 success  ax the number readed
;		   [pbSucceed]=-1 failed
ReadFile proc stdcall uses cx ds hFile:word, nCount:word, pBuff:word, pbSucceed:word
	mov ax, ss
	mov ds, ax			;used for ds:dx
	mov ah, 3fh
	mov bx, hFile
	mov cx, nCount
	mov dx, [pBuff]
	int 21h
	sbb cx, cx		;whether CF set, error occurs
	mov bl, [pbSucceed]
	mov bl, cl
	ret
ReadFile endp

WRITE_FILE:
	mov ah, 40h
	int 21h
	ret

;Function: seek to a specific position  
;Params  : hFile indicate the file handle
;		 : nHighOrder: nLowOrder indicate the position
;        : nMode indicate seek mode, can be 00(SEEK_SET), 01(SEEK_CUR), 02(SEEK_END)
;Return  : ax=0 success  ax=-1 failed
SeekFile proc stdcall hFile:word, nHighOrder:word, nLowOrder:word, nMode:byte	
	mov ah, 42h
	mov al, nMode
	mov bx, hFile
	mov cx, nHighOrder
	mov dx, nLowOrder
	int 21h
	
	;check and set status
	sbb ax, ax	
	ret
SeekFile endp
	
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