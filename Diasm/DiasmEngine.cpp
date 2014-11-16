#include <stdio.h>
#include <string.h>

//#define USE_STATIC
#ifdef USE_STATIC
	#define STATIC //static 
#else
	#define STATIC 
#endif	

//
#define  SIZE_RB  0
#define  SIZE_RW  1
#define  SIZE_SREG 2
static char gs_rb[8][3]   = {"AL", "CL", "DL", "BL","AH", "CH", "DH", "BH"};
static char gs_rw[8][3]   = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};  //SBSD
static char gs_sreg[8][3] = {"ES", "CS", "SS", "DS", "ES", "CS", "SS", "DS"};  //ECSD
static char gs_XPSDBBX[8][8] = {"BX+SI", "BX+DI", "BP+SI", "BP+DI", "SI", "DI", "BP", "BX"};	//XPSD SDBBX

#define  DIGIT_NONE -1
#define  DIGIT_0    0
#define  DIGIT_1    1
#define  DIGIT_2    2
#define  DIGIT_3    3
#define  DIGIT_4    4
#define  DIGIT_5    5
#define  DIGIT_6    6
#define  DIGIT_7    7

//
static struct _tagModRM
{
	unsigned char m_RM:3;
	unsigned char m_RegOpCode:3;
	unsigned char m_Mod:2;
}gs_tagMod;

//used for log
static char gs_szLog[64];
extern void LogInfo(const char *pszInfo);
static struct _tagLog
{
	char m_sz1stOperand[32];
	char m_sz2ndOperand[32];
	char *m_pszAction;
#define  First_Second	 0	//1st<----2nd
#define  Second_First    1   //2nd<----1st
#define  First_First     3  //only one, like push, pop
	int  m_nDirection;	//1st_2nd  2nd_1st
}gs_tagLog;


//Forward Declare
STATIC void DecodeModRM(const unsigned char *pDiasmCode, int *pnCurPos, int nSize, int nDigit);
STATIC void And2023(const unsigned char *pDiasmCode, int *pnCurPos);

STATIC void Add8081(const unsigned char *pDiasmCode, int *pnCurPos);
STATIC void Adc8081(const unsigned char *pDiasmCode, int *pnCurPos);
STATIC void Adc83(const unsigned char *pDiasmCode, int *pnCurPos);
STATIC void Sub8081(const unsigned char *pDiasmCode, int *pnCurPos);
STATIC void Sbb8081(const unsigned char *pDiasmCode, int *pnCurPos);
STATIC void Sbb83(const unsigned char *pDiasmCode, int *pnCurPos);
STATIC void Cmp8081(const unsigned char *pDiasmCode, int *pnCurPos);


/************************************************************************/
/* 
B0 + rb   mov r8, imm8
B8 + rw   mov r16, imm16
针对以上信息，可根据如下逻辑，对立即数寻址进行解析：
0）构建如下的数组
   Char const rb[8][3] = {“AL”, “CL”, “DL”, “BL”,”AH”, “CH”, “DH”, “BH”};
   Char const rw[8][3] = {“AX”,.....};
   Char const rd[8][4] = {“EAX”,....};
1）取1byte，其值为first. 
2）如果first < B0 || first > BF, 则不属于MOV的立即数寻址方式
3）如果first >= B0 && first < B8, 则为单字节 mov r8, imm8的情况
   a)用rb[first-B0] 确定寄存器。如,first=6, 则rb[6]就是”DH”
   b)再取1byte, 作为立即数输出
4）如果first >= B8 && first < C0, 则为双字节 mov r16, imm16的情况 （32位暂不考虑）
a)用rw[first-B8]确定寄存器。如,first=6,则rw[6]就是”SI”
  再取2byte,作为立即数输出（注意输出的顺序）                                                                     */
/************************************************************************/
STATIC void
MovB0B8BF(const unsigned char *pDiasmCode, int *pnCurPos)
{	
	gs_tagLog.m_pszAction  = "MOV";
	gs_tagLog.m_nDirection = First_Second;

	int nIndex = pDiasmCode[*pnCurPos] & 0x0F; //B0~BF
    if (nIndex >= 8)
    {
		  strcpy(gs_tagLog.m_sz1stOperand, gs_rw[nIndex - 8]);
		  sprintf(gs_tagLog.m_sz2ndOperand, "%.2X%.2X",  pDiasmCode[*pnCurPos + 2],
														pDiasmCode[*pnCurPos + 1]);
	
//         sprintf(gs_szLog, "MOV %s, %X%X", gs_rw[nIndex - 8], 
//                                           pDiasmCode[*pnCurPos + 2],
//                                           pDiasmCode[*pnCurPos + 1]
//                                           );

        *pnCurPos += 3;
    }
    else
    {
		strcpy(gs_tagLog.m_sz1stOperand, gs_rb[nIndex]);
		sprintf(gs_tagLog.m_sz2ndOperand, "%.2X", pDiasmCode[*pnCurPos + 1]);

//         sprintf(gs_szLog, "MOV %s, %X", gs_rb[nIndex], 
//                                           pDiasmCode[*pnCurPos + 1]
//                                           );

        *pnCurPos += 2;
    }
}

/************************************************************************/
/* 对AL, AX的特殊处理   
A0 MOV AL,moffs8        
A1 MOV AX,moffs16    A13412    MOV AX, [1234]
A2 MOV moffs8*,AL 
A3 MOV moffs16*,AX                                                   */
/************************************************************************/
STATIC void
MovA0A3(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "MOV";
	gs_tagLog.m_nDirection = First_Second;

    int nIndex = pDiasmCode[*pnCurPos] & 0x03;	//A0~A3
    if (0 == nIndex % 2)
    {
		strcpy(gs_tagLog.m_sz1stOperand, "AL");
        sprintf(gs_tagLog.m_sz2ndOperand, "[%.2X]", pDiasmCode[*pnCurPos + 1]);
		if (2 == nIndex)
		{
			gs_tagLog.m_nDirection = Second_First;
		}

        *pnCurPos += 2;
    }
    else
    {
        strcpy(gs_tagLog.m_sz1stOperand, "AX");
        sprintf(gs_tagLog.m_sz2ndOperand, "[%.2X%.2X]", pDiasmCode[*pnCurPos + 2],
														pDiasmCode[*pnCurPos + 1]);
		if (3 == nIndex)
		{
			gs_tagLog.m_nDirection = Second_First;
		}

        *pnCurPos += 3;
    } 
}

/************************************************************************/
/*  对段寄存器的处理
8C /r MOV r/m16,Sreg
8E /r MOV Sreg,r/m16                                                   */
/************************************************************************/
STATIC void
Mov8C8E(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "MOV";
	gs_tagLog.m_nDirection = First_Second;
	
    int nIndex = pDiasmCode[*pnCurPos] & 0x02;	//8C, 8E
    if (2 == nIndex)
    {
		gs_tagLog.m_nDirection = Second_First;
    }	
	
	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos + 1], 1);
	*pnCurPos += 1;
	DecodeModRM(pDiasmCode, pnCurPos, SIZE_SREG, DIGIT_NONE);
}

/************************************************************************/
/* C6 /0 MOV r/m8,imm8 
  C7 /0 MOV r/m16,imm16                                                 */
/************************************************************************/
STATIC void
MovC6C7(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "MOV";
	gs_tagLog.m_nDirection = First_Second;
	
    int nIndex = pDiasmCode[*pnCurPos] & 0x01;	//C6, C7
	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos + 1], 1);
	*pnCurPos += 1;

    if (0 == nIndex)
    {
		DecodeModRM(pDiasmCode, pnCurPos, SIZE_RB, DIGIT_0);
		sprintf(gs_tagLog.m_sz2ndOperand, "%.2X", pDiasmCode[*pnCurPos]);
		*pnCurPos += 1;
    }	
	else
	{
		DecodeModRM(pDiasmCode, pnCurPos, SIZE_RW, DIGIT_0);
		sprintf(gs_tagLog.m_sz2ndOperand, "%.2X%.2X", pDiasmCode[*pnCurPos + 1],
													pDiasmCode[*pnCurPos]);
		*pnCurPos += 2;
	}
}

/************************************************************************/
/*  
Entry point for C6 serials, 

C6 /0 MOV r/m8,imm8 
C7 /0 MOV r/m16,imm16                                                   */
/************************************************************************/
STATIC void
DispatchC6C7(const unsigned char *pDiasmCode, int *pnCurPos)
{
	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos + 1], 1);
	if (0 == gs_tagMod.m_RegOpCode)		// /0
	{
		MovC6C7(pDiasmCode, pnCurPos);
	}
	else
	{
		//U can override it.
		MovC6C7(pDiasmCode, pnCurPos);
	}
}

/************************************************************************/
/* 	
88 /r MOV r/m8,r8 
89 /r MOV r/m16,r16 
8A /r MOV r8,r/m8 
8B /r MOV r16,r/m16                                                    */
/************************************************************************/
STATIC void
Mov888B(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "MOV";
	gs_tagLog.m_nDirection = First_Second;
	
    int nIndex = pDiasmCode[*pnCurPos] & 0x03;	//88~8B
	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos + 1], 1);
	*pnCurPos += 1;

    if (0 == nIndex % 2)
    {
		DecodeModRM(pDiasmCode, pnCurPos, SIZE_RB, DIGIT_NONE);
		if (2 == nIndex)
		{
			gs_tagLog.m_nDirection = Second_First;
		}
    }
    else
    {
        DecodeModRM(pDiasmCode, pnCurPos, SIZE_RW, DIGIT_NONE);
		if (3 == nIndex)
		{
			gs_tagLog.m_nDirection = Second_First;
		}
    } 
}

/************************************************************************/
/* 
FF /6 PUSH r/m16 Push r/m16
与Mov8C8E类似，                                            */
/************************************************************************/
STATIC void
PushFF(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "PUSH";
	gs_tagLog.m_nDirection = First_First;
	
	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos + 1], 1);
	*pnCurPos += 1;	
	DecodeModRM(pDiasmCode, pnCurPos, SIZE_RW, DIGIT_6);

	return;
}

/************************************************************************/
/* 
50+rw PUSH r16 Push r16
50+rd PUSH r32 Push r32 
与MovB0B8BF类似                                               */
/************************************************************************/
STATIC void
Push5057(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "PUSH";
	gs_tagLog.m_nDirection = First_First;
	
	int nIndex = pDiasmCode[*pnCurPos] & 0x07; //50~57
	strcpy(gs_tagLog.m_sz1stOperand, gs_rw[nIndex]);	
    *pnCurPos += 1;
}

/************************************************************************/
/* 
0E PUSH CS Push CS
16 PUSH SS Push SS
1E PUSH DS Push DS
06 PUSH ES Push ES                                                      */
/************************************************************************/
STATIC void
PushSReg(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "PUSH";
	gs_tagLog.m_nDirection = First_First;

	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos], 1);
	*pnCurPos += 1;	

	//直接获取sreg即可
	strcpy(gs_tagLog.m_sz1stOperand, gs_sreg[gs_tagMod.m_RegOpCode]);
}

/************************************************************************/
/* 
8F /0 POP r/m16 Pop top of stack into m16; increment stack pointer
8F /0 POP r/m32 Pop top of stack into m32; increment stack pointer  

与PushFF类似                                                                   */
/************************************************************************/
STATIC void
Pop8F(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "POP";
	gs_tagLog.m_nDirection = First_First;

	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos], 1);
	*pnCurPos += 1;	
	DecodeModRM(pDiasmCode, pnCurPos, SIZE_RW, DIGIT_0);
}

/************************************************************************/
/* 
1F POP DS Pop top of stack into DS; increment stack pointer
07 POP ES Pop top of stack into ES; increment stack pointer
17 POP SS Pop top of stack into SS; increment stack pointer 

与PushSReg类似                                                                    */
/************************************************************************/
STATIC void
PopSReg(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "POP";
	gs_tagLog.m_nDirection = First_First;
	
	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos], 1);
	*pnCurPos += 1;	
	
	//直接获取sreg即可
	strcpy(gs_tagLog.m_sz1stOperand, gs_sreg[gs_tagMod.m_RegOpCode]);
}

/************************************************************************/
/* 
58+ rw POP r16 Pop top of stack into r16; increment stack pointer
58+ rd POP r32 Pop top of stack into r32; increment stack pointer

与Push5057类似                                                                     */
/************************************************************************/
STATIC void
Pop585F(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "POP";
	gs_tagLog.m_nDirection = First_First;
	
	int nIndex = pDiasmCode[*pnCurPos] & 0x07; //58 ~ 5F
	strcpy(gs_tagLog.m_sz1stOperand, gs_rw[nIndex]);	
    *pnCurPos += 1;
}

/************************************************************************/
/* 
C5 /r LDS r16,m16:16 Load DS:r16 with far pointer from memory
C5 /r LDS r32,m16:32 Load DS:r32 with far pointer from memory          */
/************************************************************************/
STATIC void
LDSC5(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "LDS";
	gs_tagLog.m_nDirection = Second_First;	//注意方向

	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos +1], 1);
	*pnCurPos += 1;	
	DecodeModRM(pDiasmCode, pnCurPos, SIZE_RW, DIGIT_NONE);
}

/************************************************************************/
/* 
C4 /r LES r16,m16:16 Load ES:r16 with far pointer from memory
C4 /r LES r32,m16:32 Load ES:r32 with far pointer from memory           */
/************************************************************************/
STATIC void
LESC4(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "LES";
	gs_tagLog.m_nDirection = Second_First;	//注意方向
	
	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos +1], 1);
	*pnCurPos += 1;	
	DecodeModRM(pDiasmCode, pnCurPos, SIZE_RW, DIGIT_NONE);
}

/************************************************************************/
/* 
2EH—CS segment override (use with any branch instruction is reserved).
36H—SS segment override prefix (use with any branch instruction is reserved).
3EH—DS segment override prefix (use with any branch instruction is reserved).
26H—ES segment override prefix (use with any branch instruction is reserved)

  Branch hints:
  2EH—Branch not taken (used only with Jcc instructions).
  3EH—Branch taken (used only with Jcc instructions).
																		*/                                                                   
/************************************************************************/
STATIC void
Prefix263E(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "";	//no action
	gs_tagLog.m_nDirection = First_First;	
	
	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos], 1);	//itself
	*pnCurPos += 1;	

	//直接获取sreg即可
	sprintf(gs_tagLog.m_sz1stOperand, "%s:", gs_sreg[gs_tagMod.m_RegOpCode]);
}

/************************************************************************/
/*     
24 ib AND AL,imm8 AL AND imm8
25 iw AND AX,imm16 AX AND imm16                                        */
/************************************************************************/
STATIC void
And2425(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "AND";
	gs_tagLog.m_nDirection = First_Second;
	
    int nIndex = pDiasmCode[*pnCurPos] & 0x01;	//24,25
    if (0 == nIndex)
    {
		strcpy(gs_tagLog.m_sz1stOperand, "AL");
        sprintf(gs_tagLog.m_sz2ndOperand, "%.2X", pDiasmCode[*pnCurPos + 1]);		
        *pnCurPos += 2;
    }
    else
    {
        strcpy(gs_tagLog.m_sz1stOperand, "AX");
        sprintf(gs_tagLog.m_sz2ndOperand, "%.2X%.2X", pDiasmCode[*pnCurPos + 2],
														pDiasmCode[*pnCurPos + 1]);		
        *pnCurPos += 3;
    } 
}

/************************************************************************/
/* 
80 /4 ib AND r/m8,imm8		r/m8 AND imm8
81 /4 iw AND r/m16,imm16	r/m16 AND imm16
                                                                     */
/************************************************************************/
STATIC void
And8081(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "AND";
	gs_tagLog.m_nDirection = First_Second;
	
    int nIndex = pDiasmCode[*pnCurPos] & 0x01;	//80,81
	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos + 1], 1);
	*pnCurPos += 1;

    if (0 == nIndex)
    {
		DecodeModRM(pDiasmCode, pnCurPos, SIZE_RB, DIGIT_4);
        sprintf(gs_tagLog.m_sz2ndOperand, "%.2X", pDiasmCode[*pnCurPos]);		
        *pnCurPos += 1;
    }
    else
    {
        DecodeModRM(pDiasmCode, pnCurPos, SIZE_RW, DIGIT_4);
        sprintf(gs_tagLog.m_sz2ndOperand, "%.2X%.2X", pDiasmCode[*pnCurPos + 1],
													 pDiasmCode[*pnCurPos]);		
        *pnCurPos += 2;
    } 	
}

/************************************************************************/
/* Entry point for 80,81 serials
80 /0 ib ADD r/m8,imm8 Add imm8 to r/m8
81 /0 iw ADD r/m16,imm16 Add imm16 to r/m16

80 /2 ib ADC r/m8,imm8 Add with carry imm8 to r/m8
81 /2 iw ADC r/m16,imm16 Add with carry imm16 to r/m16  

80 /4 ib AND r/m8,imm8		r/m8 AND imm8
81 /4 iw AND r/m16,imm16	r/m16 AND imm16 

80 /5 ib SUB r/m8,imm8 Subtract imm8 from r/m8
81 /5 iw SUB r/m16,imm16 Subtract imm16 from r/m16                                                                */
/************************************************************************/
STATIC void
Dispatch8081(const unsigned char *pDiasmCode, int *pnCurPos)
{
	/*
	可进步优化，如
	if(0 == gs_tagMod.m_RegOpcode)
	{
		gs_tagMod.m_szAction = "ADD";
	}
	else if (2 == gs_tagMod.m_RegOpcode)
	{
		gs_tagMod.m_szAction = "ADC";
	}
	else if....

    Add8081(pDiasmCode, pnCurPos);
	*/

	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos + 1], 1);
	if (0 == gs_tagMod.m_RegOpCode)		// /0
	{
		Add8081(pDiasmCode, pnCurPos);
	}
	else if (2 == gs_tagMod.m_RegOpCode) // /2
	{
		Adc8081(pDiasmCode, pnCurPos);
	}
	else if (3 == gs_tagMod.m_RegOpCode) // /3
	{
		Sbb8081(pDiasmCode, pnCurPos);
	}
	else if (4 == gs_tagMod.m_RegOpCode) // /4
	{
		And8081(pDiasmCode, pnCurPos);
	}
	else if (5 == gs_tagMod.m_RegOpCode) // /5
	{
		Sub8081(pDiasmCode, pnCurPos);
	}
	else if (7 == gs_tagMod.m_RegOpCode) // /7
	{
		Cmp8081(pDiasmCode, pnCurPos);
	}
	else
	{
		//
	}
}

/************************************************************************/
/* 
83 /4 ib AND r/m16,imm8 r/m16 AND imm8 (sign-extended)
83 /4 ib AND r/m32,imm8 r/m32 AND imm8 (sign-extended)                                                                     */
/************************************************************************/
STATIC void
And83(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "AND";
	gs_tagLog.m_nDirection = First_Second;
	
	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos + 1], 1);
	*pnCurPos += 1;
	
	DecodeModRM(pDiasmCode, pnCurPos, SIZE_RW, DIGIT_4);
	if (pDiasmCode[*pnCurPos] > 0x80)
	{
		sprintf(gs_tagLog.m_sz2ndOperand, "-%.2X", 0x100 - pDiasmCode[*pnCurPos]);
	}
	else
	{
		sprintf(gs_tagLog.m_sz2ndOperand, "+%.2X", pDiasmCode[*pnCurPos]);
	}
    		
    *pnCurPos += 1;
}

/************************************************************************/
/* 
3C ib CMP AL, imm8 Compare imm8 with AL
3D iw CMP AX, imm16 Compare imm16 with AX                              */
/************************************************************************/
STATIC void
Cmp3C3D(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//reuse
	And2425(pDiasmCode, pnCurPos);

	//repair
	gs_tagLog.m_pszAction = "CMP";
}

/************************************************************************/
/* 
80 /7 ib CMP r/m8, imm8 Compare imm8 with r/m8
81 /7 iw CMP r/m16, imm16 Compare imm16 with r/m16
81 /7 id CMP r/m32,imm32 Compare imm32 with r/m32                    */
/************************************************************************/
STATIC void
Cmp8081(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//reuse
	And8081(pDiasmCode, pnCurPos);

	//repair
	gs_tagLog.m_pszAction = "CMP";
}

/************************************************************************/
/* 
83 /7 ib CMP r/m16,imm8 Compare imm8 with r/m16
83 /7 ib CMP r/m32,imm8 Compare imm8 with r/m32                                                                     */
/************************************************************************/
STATIC void
Cmp83(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//reuse
	And83(pDiasmCode, pnCurPos);

	//repair
	gs_tagLog.m_pszAction = "CMP";
}

/************************************************************************/
/* 
38 /r CMP r/m8,r8 Compare r8 with r/m8
39 /r CMP r/m16,r16 Compare r16 with r/m16
3A /r CMP r8,r/m8 Compare r/m8 with r8
3B /r CMP r16,r/m16 Compare r/m16 with r16                                                                   */
/************************************************************************/
STATIC void
Cmp383B(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//reuse
	And2023(pDiasmCode, pnCurPos);
	
	//repair
	gs_tagLog.m_pszAction = "CMP";
}

/************************************************************************/
/* 
2C ib SUB AL,imm8 Subtract imm8 from AL
2D iw SUB AX,imm16 Subtract imm16 from AX

与And2425, ADC1415很像，可重用
14 ib ADC AL,imm8 Add with carry imm8 to AL
15 iw ADC AX,imm16 Add with carry imm16 to AX                         

04 ib ADD AL,imm8 Add imm8 to AL
05 iw ADD AX,imm16 Add imm16 to AX 
 
24 ib AND AL,imm8 AL AND imm8
25 iw AND AX,imm16 AX AND imm16                                         */
/************************************************************************/
STATIC void
Sub2C2D(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//reuse
	And2425(pDiasmCode, pnCurPos);

	//repair
	gs_tagLog.m_pszAction = "SUB";
}

/************************************************************************/
/*    
80 /5 ib SUB r/m8,imm8 Subtract imm8 from r/m8
81 /5 iw SUB r/m16,imm16 Subtract imm16 from r/m16                     */
/************************************************************************/
STATIC void
Sub8081(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//reuse
	And8081(pDiasmCode, pnCurPos);

	//repair
	gs_tagLog.m_pszAction = "SUB";
}

/************************************************************************/
/*
83 /5 ib SUB r/m16,imm8 Subtract sign-extended imm8 from r/m16
83 /5 ib SUB r/m32,imm8 Subtract sign-extended imm8 from r/m32                                                                     */
/************************************************************************/
STATIC void
Sub83(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//reuse
	And83(pDiasmCode, pnCurPos);

	//repair
	gs_tagLog.m_pszAction = "SUB";
}

/************************************************************************/
/*
28 /r SUB r/m8,r8 Subtract r8 from r/m8
29 /r SUB r/m16,r16 Subtract r16 from r/m16
2A /r SUB r8,r/m8 Subtract r/m8 from r8
2B /r SUB r16,r/m16 Subtract r/m16 from r16

可参考And2023,  Add0003, Adc1013 ,可重用         */
/************************************************************************/
STATIC void
Sub282B(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//Use And to implement the major work
	And2023(pDiasmCode, pnCurPos);
	
	//Then do sth repairing
	gs_tagLog.m_pszAction = "SUB";
}

/************************************************************************/
/* 
1C ib SBB AL,imm8 Subtract with borrow imm8 from AL
1D iw SBB AX,imm16 Subtract with borrow imm16 from AX

可重用Sub2C2D, And2425,.... 这些
2C ib SUB AL,imm8 Subtract imm8 from AL
2D iw SUB AX,imm16 Subtract imm16 from AX

14 ib ADC AL,imm8 Add with carry imm8 to AL
15 iw ADC AX,imm16 Add with carry imm16 to AX                         
  、
04 ib ADD AL,imm8 Add imm8 to AL
05 iw ADD AX,imm16 Add imm16 to AX 
	
24 ib AND AL,imm8 AL AND imm8
25 iw AND AX,imm16 AX AND imm16                                                              */
/************************************************************************/
STATIC void
Sbb1C1D(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//reuse
	And2425(pDiasmCode, pnCurPos);
	
	//repair
	gs_tagLog.m_pszAction = "SBB";
}

/************************************************************************/
/*
80 /3 ib SBB r/m8,imm8 Subtract with borrow imm8 from r/m8
81 /3 iw SBB r/m16,imm16 Subtract with borrow imm16 from r/m16         */
/************************************************************************/
STATIC void
Sbb8081(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//reuse
	And8081(pDiasmCode, pnCurPos);

	//repair
	gs_tagLog.m_pszAction = "SBB";
}

/************************************************************************/
/* 
83 /3 ib SBB r/m16,imm8 Subtract with borrow sign-extended imm8 from r/m16
83 /3 ib SBB r/m32,imm8 Subtract with borrow sign-extended imm8 from r/m32                                                                     */
/************************************************************************/
STATIC void
Sbb83(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//reuse
	And8081(pDiasmCode, pnCurPos);

	//
	gs_tagLog.m_pszAction = "SBB";
}

/************************************************************************/
/*   
18 /r SBB r/m8,r8 Subtract with borrow r8 from r/m8
19 /r SBB r/m16,r16 Subtract with borrow r16 from r/m16
1A /r SBB r8,r/m8 Subtract with borrow r/m8 from r8
1B /r SBB r16,r/m16 Subtract with borrow r/m16 from r16                                                                   */
/************************************************************************/
STATIC void
Sbb181B(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//reuse
	And2023(pDiasmCode, pnCurPos);

	//repair
	gs_tagLog.m_pszAction = "SBB";
}

/************************************************************************/
/* 
The Entry Point for 83
83 /2 ib ADC r/m16,imm8 Add with CF sign-extended imm8 to r/m16
83 /2 ib ADC r/m32,imm8 Add with CF sign-extended imm8 into r/m32

83 /3 ib SBB r/m16,imm8 Subtract with borrow sign-extended imm8 from r/m16
83 /3 ib SBB r/m32,imm8 Subtract with borrow sign-extended imm8 from r/m32

83 /4 ib AND r/m16,imm8 r/m16 AND imm8 (sign-extended)
83 /4 ib AND r/m32,imm8 r/m32 AND imm8 (sign-extended)   

83 /5 ib SUB r/m16,imm8 Subtract sign-extended imm8 from r/m16
83 /5 ib SUB r/m32,imm8 Subtract sign-extended imm8 from r/m32 

83 /7 ib CMP r/m16,imm8 Compare imm8 with r/m16
83 /7 ib CMP r/m32,imm8 Compare imm8 with r/m32                                                               */
/************************************************************************/
STATIC void
Dispatch83(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//可优化
	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos + 1], 1);
	if (2 == gs_tagMod.m_RegOpCode) // /2
	{
		Adc83(pDiasmCode, pnCurPos);
	}
	else if (3 == gs_tagMod.m_RegOpCode) // /3
	{
		Sbb83(pDiasmCode, pnCurPos);
	}
	else if (4 == gs_tagMod.m_RegOpCode) // /4
	{
		And83(pDiasmCode, pnCurPos);
	}
	else if (5 == gs_tagMod.m_RegOpCode) // /5
	{
		Sub83(pDiasmCode, pnCurPos);
	}
	else if (7 == gs_tagMod.m_RegOpCode) // /7
	{
		Cmp83(pDiasmCode, pnCurPos);
	}
	else
	{
		//
	}
}

/************************************************************************/
/* 
20 /r AND r/m8,r8 r/m8 AND r8
21 /r AND r/m16,r16 r/m16 AND r16
22 /r AND r8,r/m8 r8 AND r/m8
23 /r AND r16,r/m16 r16 AND r/m16                                       */
/************************************************************************/
STATIC void
And2023(const unsigned char *pDiasmCode, int *pnCurPos)
{
	gs_tagLog.m_pszAction  = "AND";
	gs_tagLog.m_nDirection = First_Second;
	
    int nIndex = pDiasmCode[*pnCurPos] & 0x03;	//20~23
	memcpy(&gs_tagMod, &pDiasmCode[*pnCurPos + 1], 1);
	*pnCurPos += 1;

    if (0 == nIndex % 2)
    {
		if (2 == nIndex)
		{
			gs_tagLog.m_nDirection = Second_First;
		}

		DecodeModRM(pDiasmCode, pnCurPos, SIZE_RB, DIGIT_NONE);
    }
    else
    {
		if (3 == nIndex)
		{
			gs_tagLog.m_nDirection = Second_First;
		}
		
		DecodeModRM(pDiasmCode, pnCurPos, SIZE_RW, DIGIT_NONE);
    } 
}

/************************************************************************/
/* 
14 ib ADC AL,imm8 Add with carry imm8 to AL
15 iw ADC AX,imm16 Add with carry imm16 to AX

与Add0405, And2425很像，可合并                           
04 ib ADD AL,imm8 Add imm8 to AL
05 iw ADD AX,imm16 Add imm16 to AX 
                                  
24 ib AND AL,imm8 AL AND imm8
25 iw AND AX,imm16 AX AND imm16*/
/************************************************************************/
STATIC void
Adc1415(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//Use And2425 to implement the major work
	And2425(pDiasmCode, pnCurPos);

	//some repairing
	gs_tagLog.m_pszAction = "ADC";
}

/************************************************************************/
/* 
80 /2 ib ADC r/m8,imm8 Add with carry imm8 to r/m8
81 /2 iw ADC r/m16,imm16 Add with carry imm16 to r/m16

与And8081, Add8081同属，可重用                                         */
/************************************************************************/
STATIC void
Adc8081(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//Use And8081 to implement the major work, 
	//As for DIGIT_0 or DIGIT_4, it doesn't matter;   But the DIGIT_NONE and !DIGIT_NONE
	And8081(pDiasmCode, pnCurPos);
	
	//And make a little change
	gs_tagLog.m_pszAction  = "ADC";	
}

/************************************************************************/
/* 
83 /2 ib ADC r/m16,imm8 Add with CF sign-extended imm8 to r/m16
83 /2 ib ADC r/m32,imm8 Add with CF sign-extended imm8 into r/m32

与And83同属，可重用                                                    */
/************************************************************************/
STATIC void
Adc83(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//Use And83 to implement the major work
	//As for DIGIT_0 or DIGIT_4, it doesn't matter;   But the DIGIT_NONE and !DIGIT_NONE
	And83(pDiasmCode, pnCurPos);
	
	//And make a little change
	gs_tagLog.m_pszAction  = "ADC";	
}

/************************************************************************/
/* 
10 /r ADC r/m8,r8 Add with carry byte register to r/m8
11 /r ADC r/m16,r16 Add with carry r16 to r/m16
12 /r ADC r8,r/m8 Add with carry r/m8 to byte register
13 /r ADC r16,r/m16 Add with carry r/m16 to r16  

与And2023, ADD0003同属，可重用                                         */
/************************************************************************/
STATIC void
Adc1013(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//reuse code
	And2023(pDiasmCode, pnCurPos);

	//repair
	gs_tagLog.m_pszAction = "ADC";
}

/************************************************************************/
/* 
00 /r ADD r/m8,r8 Add r8 to r/m8
01 /r ADD r/m16,r16 Add r16 to r/m16
02 /r ADD r8,r/m8 Add r/m8 to r8
03 /r ADD r16,r/m16 Add r/m16 to r16    

与And 20~23很像，可合并 
20 /r AND r/m8,r8 r/m8 AND r8
21 /r AND r/m16,r16 r/m16 AND r16
22 /r AND r8,r/m8 r8 AND r/m8
23 /r AND r16,r/m16 r16 AND r/m16                                */
/************************************************************************/
STATIC void
Add0003(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//Use And to implement the major work
	And2023(pDiasmCode, pnCurPos);

	//Then do sth repairing
	gs_tagLog.m_pszAction = "ADD";
}

/************************************************************************/
/* 
04 ib ADD AL,imm8 Add imm8 to AL
05 iw ADD AX,imm16 Add imm16 to AX                                   

与And2425 很像，可以进行合并。
24 ib AND AL,imm8 AL AND imm8
25 iw AND AX,imm16 AX AND imm16 */
/************************************************************************/
STATIC void
Add0405(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//Use And2425 to implement the major work
	And2425(pDiasmCode, pnCurPos);

	//then do some repairing
	gs_tagLog.m_pszAction  = "ADD";
}

/************************************************************************/
/* 
80 /0 ib ADD r/m8,imm8 Add imm8 to r/m8
81 /0 iw ADD r/m16,imm16 Add imm16 to r/m16                             
与80 /4, 81 /4 很像，可以合并*/
/************************************************************************/
STATIC void
Add8081(const unsigned char *pDiasmCode, int *pnCurPos)
{
	//Use And8081 to implement the major work, 
	//As for DIGIT_0 or DIGIT_4, it doesn't matter;   But the DIGIT_NONE and !DIGIT_NONE
	And8081(pDiasmCode, pnCurPos);

	//And make a little change
	gs_tagLog.m_pszAction  = "ADD";
}

/************************************************************************/
/* 根据gs_tagMod，解析ModR/M的映射关系
可参考IA32 VOL2 "Table 2-1. 16-Bit Addressing Forms with the ModR/M Byte"

nSize 表示displacement的大小，8, 16
nDigit 表示/digit 0~7 indicates that the ModR/M byte of the instruction uses only the r/m (register or memory) operand.
		             The reg field contains the digit that provides an extension to the instruction's opcode.
				  -1 表示无需特殊考虑
																		*/
/************************************************************************/
STATIC void
DecodeModRM(const unsigned char *pDiasmCode, int *pnCurPos, int nSize, int nDigit)
{
	int nMod = gs_tagMod.m_Mod & 0x03;
	int nRM  = gs_tagMod.m_RM  & 0x07;
	int nRegOpcode = gs_tagMod.m_RegOpCode & 0x07;

	//deal with nDigit carefully
	// imm---> memory    like MOV WORD PTR[3412], 1234   c6 /0  c7 / 0
	gs_tagLog.m_sz1stOperand[0] = '\0';
	if (DIGIT_NONE != nDigit
		&& 3 != nMod)
	{
		if (SIZE_RB == nSize)
		{
			sprintf(gs_tagLog.m_sz1stOperand, "BYTE PTR ");
		}
		else
		{
			sprintf(gs_tagLog.m_sz1stOperand, "WORD PTR ");
		}					
	}

	if (0 == nMod)
	{
		/*
		[BX+SI]
		[BX+DI]
		[BP+SI]
		[BP+DI]
		[SI]
		[DI]
		disp16
		[BX]
		*/
		if (6 == nRM)
		{
			sprintf(gs_tagLog.m_sz1stOperand, "%s[%.2X%.2X]", gs_tagLog.m_sz1stOperand,
															pDiasmCode[*pnCurPos + 2],
														    pDiasmCode[*pnCurPos + 1]);
			*pnCurPos += 3;
		}
		else
		{
			sprintf(gs_tagLog.m_sz1stOperand, "%s[%s]", gs_tagLog.m_sz1stOperand,
														gs_XPSDBBX[nRM]);
			*pnCurPos += 1;
		}
	}
	else if (1 == nMod)
	{
		/*
		[BX+SI]+disp83
		[BX+DI]+disp8
		[BP+SI]+disp8
		[BP+DI]+disp8
		[SI]+disp8
		[DI]+disp8
		[BP]+disp8
		[BX]+disp8
		*/
		//补码
		unsigned char disp8 = pDiasmCode[*pnCurPos + 1];
		if (disp8 >= 0x80)
		{
			sprintf(gs_tagLog.m_sz1stOperand, "%s[%s-%.2X]", gs_tagLog.m_sz1stOperand,
															gs_XPSDBBX[nRM],
														   0x100 - disp8);
		}
		else
		{
			sprintf(gs_tagLog.m_sz1stOperand, "%s[%s+%.2X]", gs_tagLog.m_sz1stOperand,
															gs_XPSDBBX[nRM],
														   disp8);
		}

		*pnCurPos += 2;

	}
	else if (2 == nMod)
	{
		/*
		[BX+SI]+disp16
		[BX+DI]+disp16
		[BP+SI]+disp16
		[BP+DI]+disp16
		[SI]+disp16
		[DI]+disp16
		[BP]+disp16
		[BX]+disp16
		*/
		sprintf(gs_tagLog.m_sz1stOperand, "%s[%s+%.2X%.2X]", gs_tagLog.m_sz1stOperand,
															gs_XPSDBBX[nRM],
														   pDiasmCode[*pnCurPos + 2],
														   pDiasmCode[*pnCurPos + 1]);
		*pnCurPos += 3;
	}
	else if (3 == nMod)
	{
		/*
		EAX/AX/AL/MM0/XMM0
		ECX/CX/CL/MM1/XMM1
		EDX/DX/DL/MM2/XMM2
		EBX/BX/BL/MM3/XMM3
		ESP/SP/AHMM4/XMM4
		EBP/BP/CH/MM5/XMM5
		ESI/SI/DH/MM6/XMM6
		EDI/DI/BH/MM7/XMM7
		*/
		if (SIZE_RB == nSize)
		{
			sprintf(gs_tagLog.m_sz1stOperand, "%s", gs_rb[nRM]);	
		}
		else //treate SIZE_RW, SIZE_SREG be the same
		{
			sprintf(gs_tagLog.m_sz1stOperand, "%s", gs_rw[nRM]);	
		}

		*pnCurPos += 1;
	}

	//仅根据ModR/M取一个operand
	if (DIGIT_NONE != nDigit)
	{
		return;
	}

	//还可以对数组进行优化, gs_rb, gs_rw, gs_sreg ===> gs_r[2][8][3] = {gs_rb, gs_rw, gs_sreg}
	if (SIZE_RB == nSize)
	{
		sprintf(gs_tagLog.m_sz2ndOperand, "%s", gs_rb[nRegOpcode]);	
	}
	else if (SIZE_RW == nSize)
	{
		sprintf(gs_tagLog.m_sz2ndOperand, "%s", gs_rw[nRegOpcode]);	
	}
	else if (SIZE_SREG == nSize)
	{
		sprintf(gs_tagLog.m_sz2ndOperand, "%s", gs_sreg[nRegOpcode]);
	}
}

/************************************************************************/
/* 对pDiasmCode[*pnCurPos]进行分类解码，并记录结果到gs_szLog中         */
/************************************************************************/
STATIC void
Decode(const unsigned char *pDiasmCode, int *pnCurPos, int nLen)
{
	//////////////////////////////////////////////////////////////////////////
	//MOV 系列

    /*
    B0 + rb   mov r8, imm8
    B8 + rw   mov r16, imm16
    */
    if (0xB0 == (pDiasmCode[*pnCurPos] & 0xF0))
    {
        MovB0B8BF(pDiasmCode, pnCurPos);
        return;
    }

    /*
    A0 MOV AL,moffs8
    A1 MOV AX,moffs16
    A2 MOV moffs8*,AL 
    A3 MOV moffs16*,AX 
    */
    if (0xA0 == (pDiasmCode[*pnCurPos] & 0xFC))
    {
		MovA0A3(pDiasmCode, pnCurPos);
        return;
    }  
	
	/*
	8C /r MOV r/m16,Sreg 
	8E /r MOV Sreg,r/m16 
	*/
	if (0x8C == (pDiasmCode[*pnCurPos] & 0xFD))
	{
		Mov8C8E(pDiasmCode, pnCurPos);
		return;
	}

	/*
	C6 /0 MOV r/m8,imm8 
	C7 /0 MOV r/m16,imm16 
	*/
	if (0xC6 == (pDiasmCode[*pnCurPos] & 0xFE))
	{
		//MovC6C7(pDiasmCode, pnCurPos);
		DispatchC6C7(pDiasmCode, pnCurPos);
		return;
	}

	/*
	88 /r MOV r/m8,r8 
	89 /r MOV r/m16,r16 
	8A /r MOV r8,r/m8 
	8B /r MOV r16,r/m16 
	*/
	if (0x88 == (pDiasmCode[*pnCurPos] & 0xFC))
	{
		Mov888B(pDiasmCode, pnCurPos);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//And 系列

	/*
	24 ib AND AL,imm8 AL AND imm8
	25 iw AND AX,imm16 AX AND imm16
	*/
	if (0x24 == (pDiasmCode[*pnCurPos] & 0xFE))
	{
		And2425(pDiasmCode, pnCurPos);
		return;
	}

	/*
	80 /0 ib ADD r/m8,imm8 Add imm8 to r/m8
	81 /0 iw ADD r/m16,imm16 Add imm16 to r/m16

	80 /2 ib ADC r/m8,imm8 Add with carry imm8 to r/m8
	81 /2 iw ADC r/m16,imm16 Add with carry imm16 to r/m16  

    80 /3 ib SBB r/m8,imm8 Subtract with borrow imm8 from r/m8
    81 /3 iw SBB r/m16,imm16 Subtract with borrow imm16 from r/m16

	80 /4 ib AND r/m8,imm8		r/m8 AND imm8
	81 /4 iw AND r/m16,imm16	r/m16 AND imm16 

	80 /5 ib SUB r/m8,imm8 Subtract imm8 from r/m8
	81 /5 iw SUB r/m16,imm16 Subtract imm16 from r/m16

    80 /7 ib CMP r/m8, imm8 Compare imm8 with r/m8
    81 /7 iw CMP r/m16, imm16 Compare imm16 with r/m16    
	*/
	if (0x80 == (pDiasmCode[*pnCurPos] & 0xFE))
	{
		//And8081(pDiasmCode, pnCurPos);
		Dispatch8081(pDiasmCode, pnCurPos);
		return;
	}

	/*
	83 /4 ib AND r/m16,imm8 r/m16 AND imm8 (sign-extended)
	83 /4 ib AND r/m32,imm8 r/m32 AND imm8 (sign-extended)

    83 /2 ib ADC r/m16,imm8 Add with CF sign-extended imm8 to r/m16
    83 /2 ib ADC r/m32,imm8 Add with CF sign-extended imm8 into r/m32

    83 /3 ib SBB r/m16,imm8 Subtract with borrow sign-extended imm8 from r/m16
    83 /3 ib SBB r/m32,imm8 Subtract with borrow sign-extended imm8 from r/m32

    83 /7 ib CMP r/m16,imm8 Compare imm8 with r/m16
    83 /7 ib CMP r/m32,imm8 Compare imm8 with r/m32
	*/
	if (0x83 == (pDiasmCode[*pnCurPos]))
	{
		//And83(pDiasmCode, pnCurPos);
		Dispatch83(pDiasmCode, pnCurPos);
		return;
	}

	/*
	20 /r AND r/m8,r8 r/m8 AND r8
	21 /r AND r/m16,r16 r/m16 AND r16
	22 /r AND r8,r/m8 r8 AND r/m8
	23 /r AND r16,r/m16 r16 AND r/m16
	*/
	if (0x20 == (pDiasmCode[*pnCurPos] & 0xFC))
	{
		And2023(pDiasmCode, pnCurPos);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//ADD系列

	/*
	04 ib ADD AL,imm8 Add imm8 to AL
	05 iw ADD AX,imm16 Add imm16 to AX
	*/
	if (0x04 == (pDiasmCode[*pnCurPos] & 0xFE))
	{
		Add0405(pDiasmCode, pnCurPos);
		return;
	}

	/*
	80 /0 ib ADD r/m8,imm8 Add imm8 to r/m8
	81 /0 iw ADD r/m16,imm16 Add imm16 to r/m16
	See above 	  
	*/

	/*
	00 /r ADD r/m8,r8 Add r8 to r/m8
	01 /r ADD r/m16,r16 Add r16 to r/m16
	02 /r ADD r8,r/m8 Add r/m8 to r8
	03 /r ADD r16,r/m16 Add r/m16 to r16
	*/
	if (0x00 == (pDiasmCode[*pnCurPos] & 0xFC))
	{
		Add0003(pDiasmCode, pnCurPos);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//ADC系列

	/*
	14 ib ADC AL,imm8 Add with carry imm8 to AL
	15 iw ADC AX,imm16 Add with carry imm16 to AX
	*/
	if (0x14 == (pDiasmCode[*pnCurPos] & 0xFE))
	{
		Adc1415(pDiasmCode, pnCurPos);
		return;
	}

	/*
	10 /r ADC r/m8,r8 Add with carry byte register to r/m8
	11 /r ADC r/m16,r16 Add with carry r16 to r/m16
	12 /r ADC r8,r/m8 Add with carry r/m8 to byte register
	13 /r ADC r16,r/m16 Add with carry r/m16 to r16
	*/
	if (0x10 == (pDiasmCode[*pnCurPos] & 0xFC))
	{
		Adc1013(pDiasmCode, pnCurPos);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//SUB 系列
	/*
	2C ib SUB AL,imm8 Subtract imm8 from AL
	2D iw SUB AX,imm16 Subtract imm16 from AX
	2D id SUB EAX,imm32 Subtract imm32 from EAX
	*/
	if (0x2C == (pDiasmCode[*pnCurPos] & 0xFE))
	{
		Sub2C2D(pDiasmCode, pnCurPos);
		return;
	}

	/*
	*/
	if (0x28 == (pDiasmCode[*pnCurPos] & 0xFC))
	{
		Sub282B(pDiasmCode, pnCurPos);
		return;
	}
	
	/*
	80 /5 ib SUB r/m8,imm8 Subtract imm8 from r/m8
	81 /5 iw SUB r/m16,imm16 Subtract imm16 from r/m16
	81 /5 id SUB r/m32,imm32 Subtract imm32 from r/m32
	See above
	*/

	/*
	1C ib SBB AL,imm8 Subtract with borrow imm8 from AL
	1D iw SBB AX,imm16 Subtract with borrow imm16 from AX
	*/
	if (0x1C == (pDiasmCode[*pnCurPos] & 0xFE))
	{
		Sbb1C1D(pDiasmCode, pnCurPos);
		return;
	}

	/*
	18 /r SBB r/m8,r8 Subtract with borrow r8 from r/m8
	19 /r SBB r/m16,r16 Subtract with borrow r16 from r/m16
	1A /r SBB r8,r/m8 Subtract with borrow r/m8 from r8
	1B /r SBB r16,r/m16 Subtract with borrow r/m16 from r16
	*/
	if (0x18 == (pDiasmCode[*pnCurPos] & 0xFC))
	{
		Sbb181B(pDiasmCode, pnCurPos);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//CMP 系列
	
	/*
	3C ib CMP AL, imm8 Compare imm8 with AL
	3D iw CMP AX, imm16 Compare imm16 with AX
	*/
	if (0x3C == (pDiasmCode[*pnCurPos] & 0xFE))
	{
		Cmp3C3D(pDiasmCode, pnCurPos);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//Push 系列

	/*
	FF /6 PUSH r/m16 Push r/m16
	FF /6 PUSH r/m32 Push r/m32
	*/
	if (0xFF == (pDiasmCode[*pnCurPos]))
	{
		PushFF(pDiasmCode, pnCurPos);
		return;
	}

	/*
	50+rw PUSH r16 Push r16
	50+rd PUSH r32 Push r32
	*/
	if (0x50 == (pDiasmCode[*pnCurPos] & 0xF8))
	{
		Push5057(pDiasmCode, pnCurPos);
		return;
	}

	/*
	6A PUSH imm8 Push imm8
	68 PUSH imm16 Push imm16
	68 PUSH imm32 Push imm32

	ERROR
	*/

	/*
	0F A0 PUSH FS Push FS
	0F A8 PUSH GS Push GS

	ERROR
	*/

	/*
	0E PUSH CS Push CS
	16 PUSH SS Push SS
	1E PUSH DS Push DS
	06 PUSH ES Push ES
	*/
	if (0x06 == (pDiasmCode[*pnCurPos] & 0xE7))
	{
		PushSReg(pDiasmCode, pnCurPos);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//Pop系列

	/*
	8F /0 POP r/m16 Pop top of stack into m16; increment stack pointer
	8F /0 POP r/m32 Pop top of stack into m32; increment stack pointer
	*/
	if (0x8F == (pDiasmCode[*pnCurPos]))
	{
		Pop8F(pDiasmCode, pnCurPos);
		return;
	}

	/*
	58+ rw POP r16 Pop top of stack into r16; increment stack pointer
	58+ rd POP r32 Pop top of stack into r32; increment stack pointer
	*/
	if (0x58 == (pDiasmCode[*pnCurPos] & 0xF8))
	{
		Pop585F(pDiasmCode, pnCurPos);
		return;
	}

	/*
	1F POP DS Pop top of stack into DS; increment stack pointer
	07 POP ES Pop top of stack into ES; increment stack pointer
	17 POP SS Pop top of stack into SS; increment stack pointer
	*/
	if (0x07 == (pDiasmCode[*pnCurPos] & 0xE7)
		&& 0x0F != pDiasmCode[*pnCurPos])
	{
		PopSReg(pDiasmCode, pnCurPos);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//LES, LDS, LSS
	/*
	C5 /r LDS r16,m16:16 Load DS:r16 with far pointer from memory
	C5 /r LDS r32,m16:32 Load DS:r32 with far pointer from memory
	*/
	if (0xC5 == (pDiasmCode[*pnCurPos]))
	{
		LDSC5(pDiasmCode, pnCurPos);
		return;
	}

	/*
	C4 /r LES r16,m16:16 Load ES:r16 with far pointer from memory
	C4 /r LES r32,m16:32 Load ES:r32 with far pointer from memory
	*/
	if (0xC4 == (pDiasmCode[*pnCurPos]))
	{
		LESC4(pDiasmCode, pnCurPos);
		return;
	}

	/*
	段跨越前缀  待完善
	2EH—CS segment override (use with any branch instruction is reserved).
	36H—SS segment override prefix (use with any branch instruction is reserved).
	3EH—DS segment override prefix (use with any branch instruction is reserved).
	26H—ES segment override prefix (use with any branch instruction is reserved)

  Branch hints:
	2EH—Branch not taken (used only with Jcc instructions).
	3EH—Branch taken (used only with Jcc instructions).
	*/
	if (0x26 == (pDiasmCode[*pnCurPos] & 0xE7))
	{
		Prefix263E(pDiasmCode, pnCurPos);
		return;
	}

	//TBD, quick end
	*pnCurPos += nLen;
}

/************************************************************************/
/* Format gs_tagLog to gs_szLog, used for Log                          */
/************************************************************************/
STATIC void
PreLog(const unsigned char *pDiasmCode, int nCurPos, int nPrePos)
{
	sprintf(gs_szLog, "%08X  ", nPrePos);
	for (int i = nPrePos; i < nCurPos; i++)
	{
		sprintf(gs_szLog, "%s%.2X", gs_szLog, pDiasmCode[i]);
	}
	strcat(gs_szLog, "\t\t");

	if (First_Second == gs_tagLog.m_nDirection)
	{
		sprintf(gs_szLog, "%s%s %s, %s", gs_szLog,
									    gs_tagLog.m_pszAction,
										gs_tagLog.m_sz1stOperand,
										gs_tagLog.m_sz2ndOperand);
	}
	else if (Second_First == gs_tagLog.m_nDirection)
	{
		sprintf(gs_szLog, "%s%s %s, %s", gs_szLog,
										gs_tagLog.m_pszAction,
									    gs_tagLog.m_sz2ndOperand,
										gs_tagLog.m_sz1stOperand);
	}
	else if (First_First == gs_tagLog.m_nDirection)
	{
		sprintf(gs_szLog, "%s%s %s", gs_szLog,
								     gs_tagLog.m_pszAction,
									  gs_tagLog.m_sz1stOperand);
	}
	else
	{
		//Sth wrong
		
	}
	strcat(gs_szLog, "\r\n");
}

/************************************************************************/
/* Diasm Entry
Param   : pDiasmCode points to the buffer to be diasmed
          nLen is the length
                                                                       */
/************************************************************************/
void 
DiasmEntry(const char *pDiasmCode, int nLen)
{
    int nCurPos = 0;
	int nPrePos = 0;
    while (nCurPos < nLen)
    {
		nPrePos = nCurPos;
        Decode((const unsigned char *)pDiasmCode, &nCurPos, nLen);
		PreLog((const unsigned char *)pDiasmCode, nCurPos, nPrePos);
        LogInfo(gs_szLog);
    }
}