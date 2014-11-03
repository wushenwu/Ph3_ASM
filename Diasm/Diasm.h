// Diasm.h : main header file for the DIASM application
//

#if !defined(AFX_DIASM_H__E35BB8BA_64DF_4F80_974D_EEE2BD004CDD__INCLUDED_)
#define AFX_DIASM_H__E35BB8BA_64DF_4F80_974D_EEE2BD004CDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDiasmApp:
// See Diasm.cpp for the implementation of this class
//

class CDiasmApp : public CWinApp
{
public:
	CDiasmApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiasmApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDiasmApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIASM_H__E35BB8BA_64DF_4F80_974D_EEE2BD004CDD__INCLUDED_)
