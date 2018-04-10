﻿// HustBase.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "HustBase.h"

#include "MainFrm.h"
#include "HustBaseDoc.h"
#include "HustBaseView.h"
#include "TreeList.h"

#include "IX_Manager.h"
#include "PF_Manager.h"
#include "RM_Manager.h"
#include "SYS_Manager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHustBaseApp

BEGIN_MESSAGE_MAP(CHustBaseApp, CWinApp)
	//{{AFX_MSG_MAP(CHustBaseApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_CREATEDB, OnCreateDB)
	ON_COMMAND(ID_OPENDB, OnOpenDB)
	ON_COMMAND(ID_DROPDB, OnDropDb)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHustBaseApp construction

CHustBaseApp::CHustBaseApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CHustBaseApp object

CHustBaseApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CHustBaseApp initialization
bool CHustBaseApp::pathvalue=false;

BOOL CHustBaseApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CHustBaseDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CHustBaseView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}


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
		// No message handlers
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

// App command to run the dialog
void CHustBaseApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CHustBaseApp message handlers

void CHustBaseApp::OnCreateDB()
{
	//关联创建数据库按钮，此处应提示用户输入数据库的存储路径和名称，并调用CreateDB函数创建数据库。

	//详情：弹出文件框
	BROWSEINFO bi;
	LPITEMIDLIST lpDlist = NULL;
	char szPath[MAX_PATH];
	char *dbPath, *dbName;
	CString str;
	RC rc;

	SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &lpDlist);
	if (lpDlist == NULL) return;

	ZeroMemory(&bi, sizeof(BROWSEINFO));//不存在这句就会出错
	bi.hwndOwner = GetForegroundWindow();//父窗口	
	bi.pidlRoot = lpDlist;
	bi.lpszTitle = "另存为"; bi.pszDisplayName = szPath;
	bi.ulFlags = 0x0040;//通过对话框的“新建文件夹“输入文件夹名称

	lpDlist = SHBrowseForFolder(&bi);
	if (lpDlist != NULL)
	{
		SHGetPathFromIDList(lpDlist, str.GetBuffer(MAX_PATH * 2));//把文件夹路径取出来
		str.ReleaseBuffer();

		dbPath = str.GetBuffer(0); dbName = szPath;
		rc = CreateDB(dbPath, dbName);
		if (rc != SUCCESS)return;
	}
}

void CHustBaseApp::OnOpenDB()
{
	//关联打开数据库按钮，此处应提示用户输入数据库所在位置，并调用OpenDB函数改变当前数据库路径，并在界面左侧的控件中显示数据库中的表、列信息。

	/*
	详情：把当前路径定位到要打开的数据库所在的文件夹（SetCurrentDirectory），然后读取该文件夹中SYSTABLES和SYSCOLUMNS文件的内容，
	在界面左侧显示当前数据库的结构。显示功能通过调用PopulateTree()函数来实现，具体语句为：
						CHustBaseDoc *pDoc;
					pDoc = CHustBaseDoc::GetDoc();
					CHustBaseApp::pathvalue = true;
						pDoc->m_pTreeView->PopulateTree();
	*/
	LPITEMIDLIST lpDlist;
	BROWSEINFO bi;
	RC rc;
	CHAR path[MAX_PATH];

	SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &lpDlist);
	if (lpDlist == NULL) {
		return;
	}
	ZeroMemory(&bi, sizeof(bi));
	bi.pidlRoot = lpDlist; // 文件夹对话框之根目录，不指定的话则为我的电脑
	lpDlist = SHBrowseForFolder(&bi);
	if (lpDlist != NULL) {
		SHGetPathFromIDList(lpDlist, path);
	}

	SetCurrentDirectory(path);
	CFileFind fileFind;
	BOOL table_Exist = (BOOL)fileFind.FindFile("SYSTABLES");
	BOOL columns_Exist = (BOOL)fileFind.FindFile("SYSCOLUMNS");
	if (!table_Exist || !columns_Exist)
	{
		AfxMessageBox("文件格式错误！");
		return;
	}
	CHustBaseApp::pathvalue = true;
	CHustBaseDoc *pDoc;
	pDoc = CHustBaseDoc::GetDoc();
	pDoc->m_pTreeView->PopulateTree();
	rc = OpenDB(path);
	if (rc != SUCCESS) {
		return;
	}
}

void CHustBaseApp::OnDropDb()
{
	//关联删除数据库按钮，此处应提示用户输入数据库所在位置，并调用DropDB函数删除数据库的内容。

	//详情：删除指定数据库所在文件夹中的所有数据库文件 即直接删除该数据库文件夹
	BROWSEINFO bi;
	LPITEMIDLIST lpDlist = NULL;
	char szPath[MAX_PATH];
	char *dbName;
	RC rc;

	SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &lpDlist);
	if (lpDlist == NULL) return;

	ZeroMemory(&bi, sizeof(BROWSEINFO));//不存在这句就会出错
	bi.hwndOwner = GetForegroundWindow();//父窗口	
	bi.pidlRoot = lpDlist;
	bi.lpszTitle = "删除数据库";
	bi.pszDisplayName = szPath;

	lpDlist = SHBrowseForFolder(&bi);
	if (lpDlist != NULL)
	{
		SHGetPathFromIDList(lpDlist, szPath);//把文件夹路径取出来

		CHustBaseApp::pathvalue = false; dbName = szPath;
		rc = DropDB(dbName);
		if (rc != SUCCESS)return;
	}
}
