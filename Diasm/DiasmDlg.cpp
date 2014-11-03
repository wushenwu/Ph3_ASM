// DiasmDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Diasm.h"
#include "DiasmDlg.h"

#include "DiasmEngine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiasmDlg dialog

//////////////////////////////////////////////////////////////////////////
//used for Log
static CDiasmDlg *gs_pDlg;
static char *gs_pFileBuff;    //file content
static long gs_lFileSize = 0;

CDiasmDlg::CDiasmDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDiasmDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDiasmDlg)
	m_strDiasm = _T("");
	m_strASM = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);  
    
    //
    gs_pDlg = this;
	gs_pFileBuff = NULL;

	m_bFile = FALSE;
}

void CDiasmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDiasmDlg)
	DDX_Control(pDX, IDC_OUTPUT, m_EditOutput);
	DDX_Text(pDX, IDC_DIASM, m_strDiasm);
	DDX_Text(pDX, IDC_ASM, m_strASM);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDiasmDlg, CDialog)
	//{{AFX_MSG_MAP(CDiasmDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_DIASM, OnBtnDiasm)
	ON_BN_CLICKED(IDC_BTN_OPENFILE, OnBtnOpenfile)
	ON_BN_CLICKED(IDC_BTN_DIASM_FILE, OnBtnDiasmFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiasmDlg message handlers

BOOL CDiasmDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDiasmDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDiasmDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDiasmDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDiasmDlg::OnBtnDiasm() 
{
	//For file
	m_bFile = FALSE;

    //get input
   	UpdateData();

    //illegal check
    int nLen = m_strDiasm.GetLength();
    if (0 == nLen
        || nLen % 2 != 0)
    {
        return;
    }

    //diasm now, but pre-process needed, convert "B012" to 0xB012
	char *pBuff = NULL;
	pBuff = (char *)new char[nLen * 2];
	if (NULL == pBuff)
	{
		return;
	}

    char *pSRC   = m_strDiasm.GetBuffer(0);
    char pTmp[3] = {0};

	nLen = nLen / 2;
    for (int i = 0; i < nLen; i++)
    {
        strncpy(pTmp, &pSRC[i * 2], 2);
        sscanf(pTmp, "%X", &pBuff[i]);
    }

    DiasmEntry(pBuff, nLen);  
	
	if (pBuff != NULL)
	{
		delete[] pBuff;
		pBuff = NULL;
	}
}

static int LoadFile(char *pszFileName);

void CDiasmDlg::OnBtnOpenfile() 
{
    CFileDialog *pDlgSRCFile = new CFileDialog(TRUE);
    if (NULL == pDlgSRCFile)
    {
        return;
    }
    
    if (pDlgSRCFile->DoModal() != IDOK)
    {
        return;
    }  
    
    //显示文件路径
    CString csPath = pDlgSRCFile->GetPathName();
    GetDlgItem(IDC_FILE)->SetWindowText(csPath);

	//
	LoadFile(csPath.GetBuffer(0));
}

void CDiasmDlg::OnBtnDiasmFile() 
{
	//For log
	m_bFile = TRUE;
	m_EditOutput.SetSel(0, -1);
	m_EditOutput.Clear();

	//
	DiasmEntry(gs_pFileBuff, gs_lFileSize);
}

/************************************************************************/
/* LogInfo                                                             */
/************************************************************************/
void LogInfo(const char *pszInfo)
{
    gs_pDlg->m_strASM = pszInfo;

    gs_pDlg->Log();
}

void CDiasmDlg::Log()
{
	if (!m_bFile)
	{
		UpdateData(FALSE);
	}
	else
	{
		m_EditOutput.SetSel(-1);
		m_EditOutput.ReplaceSel(gs_pDlg->m_strASM);
	}    
}

/************************************************************************/
/* Try to load file into memory,
   return SUCCESS if okay
   return BAD_PARAM if pszFileName is NULL
   return NOT_EXIST if open file error
   return FAILURE for other reason

*/
/************************************************************************/
#define SUCCESS     0x00
#define NOT_EXIST   0x01
#define BAD_PARAM   0x02
#define FAILURE      0x03    //other reason

static int
LoadFile(char *pszFileName)
{
    FILE *fp      = NULL;
    int nRet      = SUCCESS;
    char *pszInfo = "LoadFile Success";

    if (NULL == pszFileName)
    {
        nRet    = BAD_PARAM;
        pszInfo = "Bad Param For File Name Provided";
        goto END;
    }

    fp = fopen(pszFileName, "rb");
    if (NULL == fp)
    {
        nRet    = NOT_EXIST;
        pszInfo = "Open the File FAILURE";
        goto END;
    }

    //get file size
    nRet = fseek(fp, 0, SEEK_END);
    if (nRet)
    {
        nRet    = FAILURE;
        pszInfo = "fseek to the end of file FAILURE";
        goto END;
    }

    gs_lFileSize = ftell(fp);
    if (-1L == gs_lFileSize)
    {
        nRet    = FAILURE;
        pszInfo = "ftell FAILURE";
        goto END;
    }

    //rollback
    nRet = fseek(fp, 0, SEEK_SET);
    if (nRet)
    {
        nRet    = FAILURE;
        pszInfo = "fseek to the beginning of file FAILURE";
        goto END;
    }

    //alloc mem and load file into
    gs_pFileBuff = (char *)malloc(gs_lFileSize);
    if (NULL == gs_pFileBuff)
    {
        nRet    = FAILURE;
        pszInfo = "malloc of file size FAILURE";
        goto END;
    }

    fread(gs_pFileBuff, sizeof(char), gs_lFileSize, fp);
    if(ferror(fp))
    {
        nRet    = FAILURE;
        pszInfo = "load into mem FAILURE";
        goto END;
    }

END:
    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }

    LogInfo(pszInfo);
    return nRet;
}
