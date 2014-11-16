// DiasmDlg.h : header file
//

#if !defined(AFX_DIASMDLG_H__2516F2D9_E537_4D52_9F24_2F5D4BB9EA8C__INCLUDED_)
#define AFX_DIASMDLG_H__2516F2D9_E537_4D52_9F24_2F5D4BB9EA8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDiasmDlg dialog

class CDiasmDlg : public CDialog
{
// Construction
public:
	void Log();
	CDiasmDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDiasmDlg)
	enum { IDD = IDD_DIASM_DIALOG };
	CEdit	m_EditOutput;
	CString	m_strDiasm;
	CString	m_strASM;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiasmDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	BOOL m_bFile;	//是否在处理File

	// Generated message map functions
	//{{AFX_MSG(CDiasmDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnDiasm();
	afx_msg void OnBtnOpenfile();
	afx_msg void OnBtnDiasmFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIASMDLG_H__2516F2D9_E537_4D52_9F24_2F5D4BB9EA8C__INCLUDED_)
