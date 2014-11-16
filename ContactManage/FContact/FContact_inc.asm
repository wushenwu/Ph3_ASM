ifndef FCONTACT_INC
FCONTACT_INC equ  0

;-----------------Structures here--------------------
;structure used to get input for name, tel
tagRecordInput struct
	m_nNameBufLen  db ?			;should set to 20h, the max len for name
	m_nNameBufUsed db ?			;the actual len for name
	m_strName	   db 20h dup(?);name from input stored here
	
	m_nTelBufLen   db ?
	m_nTelBufUsed  db ?
	m_strTel       db 10h dup(?)
tagRecordInput ends

;structure used to store name, tel read from file
tagRecordRead struct
	m_bDeleted	   db ?
	m_strName	   db 20h dup(?)
	m_strTel       db 10h dup(?)
tagRecordRead ends
	
;-----------------Control--------------------	
;add statements for your subroutines here
;Function: init for ds, es,ss
;Params  : None
;Return  : None
SegInit proto stdcall

;Function: Try to open the file; create it if not exist
;Params  : pointer to file name
;Return  : ax=0 if success; otherwise ax=-1
LoadTables proto stdcall pstrInfoFile:word

;Function: Dispatch according to user's choice
;Params  : byte nChoice as the user's choice
;Return  : None
Dispatch proto stdcall nChoice:byte

;Function: replace OD with '$' to end the buf
;Params  : word ptr pBuff points to the buf, 
;		   nLen is the OD's index
;Return  : None
ClearOD proto stdcall pBuff:word, nLen:byte

;------------------File Operation--------------------
;Function: try to open the file specified by pstrFileName
;Params  : word ptr pstrFileName  points to filename
;		   word ptr pbSucceed  used to receive the status
;Return  : if success, [pbSucceed] set to 0, and ax as filehandle
;             failed, [pbSucceed] set to -1
OpenFile proto stdcall pstrFileName:word, pbSucceed:word

;Function: try to create file specified by pstrFileName
;Params  : word ptr pstrFileName points to filename
;			word ptr pbSucceed  used to receive the status
;Return	 : if success, [pbSucceed] set to 0, and ax as filehandle
;             failed, [pbSucceed] set to -1
CreateFile proto stdcall pstrFileName:word, pbSucceed:word

;------------------User Input-------------------------

;Function: show info pointed by pstrInfo
;Params  : word ptr pstrInfo points to the info to be shown
;Return  : None		
ShowMsg proto stdcall pstrInfo:word

;Function: get user's selection for Main Menu
;Params  : None
;Return  : al as the choice
GetSelect proto stdcall

;Function: get one byte input from user
;Params  : None
;Return  : al as the input
GetOneInput proto stdcall

;Function: Buffered Keyboard Input
;Params  : word ptr pBuff points to the buf to receive data
;Return  : None
GetBufInput proto stdcall pBuff:word

;Function: Get info about name, tel, and store into buf pointed by pRecordInput,
;		   used for Add, Search, and Modify
;Params  : word ptr pRecordInput is the buf to receive the input
;Return  : None
GetContactInfo proto stdcall pRecordInput:word



endif