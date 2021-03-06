
// CommTest.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "CommTest.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "CommTestDoc.h"
#include "CommTestView.h"

#include "ClientChildFrm.h"
#include "ClientCommDoc.h"  
#include "ClientOptionView.h"

#include "PingChildFrm.h"
#include "PingTestDoc.h"
#include "PingOptionView.h"

#include "SvrChildFrm.h"
#include "SvrCommDoc.h"
#include "SvrOptionView.h"

#include "LogDataApi.h"
#include  "boost/memory.hpp"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCommTestApp

BEGIN_MESSAGE_MAP(CCommTestApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CCommTestApp::OnAppAbout)
END_MESSAGE_MAP()


// CCommTestApp 构造

CCommTestApp::CCommTestApp() :
CBCGPWorkspace (TRUE /* m_bResourceSmartUpdate */)
{

	m_bHiColorIcons = TRUE;
	m_pDocTemplate = NULL;
	m_pPingDocTemplate = NULL;
	m_pClientDocTemplate = NULL;
	m_pSvrDocTemplate  = NULL;
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中

	NS_BOOST_MEMORY::enableMemoryLeakCheck();

}

// 唯一的一个 CCommTestApp 对象

CCommTestApp theApp;


// CCommTestApp 初始化

BOOL CCommTestApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	globalData.SetDPIAware ();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("CommTestStudio"));
//	LoadStdProfileSettings(4);  // 加载标准 INI 文件选项(包括 MRU)
	SetRegistryBase (_T("Settings"));

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CBCGPToolTipParams ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(BCGP_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CBCGPToolTipCtrl), &ttParams);

	// 注册应用程序的文档模板。文档模板
	// 将用作文档、框架窗口和视图之间的连接
	RegisterLog( _T("MAIN"));

	m_pDocTemplate = new CMultiDocTemplate(IDR_CommTestTYPE,
		RUNTIME_CLASS(CCommTestDoc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CCommTestView));
	if (!m_pDocTemplate)
		return FALSE;
	AddDocTemplate(m_pDocTemplate);

	m_pPingDocTemplate = new CMultiDocTemplate(IDR_PING_TEST,
		RUNTIME_CLASS(CPingTestDoc),
		RUNTIME_CLASS(CPingChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CPingOptionView));
	if (!m_pPingDocTemplate)
		return FALSE;
	AddDocTemplate(m_pPingDocTemplate);

	m_pClientDocTemplate = new CMultiDocTemplate(IDR_CLIENT_COMM,
		RUNTIME_CLASS(CClientCommDoc),
		RUNTIME_CLASS(CClientChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CClientOptionView));
	if (!m_pClientDocTemplate)
		return FALSE;

	AddDocTemplate(m_pClientDocTemplate);

	m_pSvrDocTemplate = new CMultiDocTemplate(IDR_SVR_COMM,
		RUNTIME_CLASS(CSvrCommDoc),
		RUNTIME_CLASS(CSvrChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CSvrOptionView));
	if (!m_pSvrDocTemplate)
		return FALSE;

	AddDocTemplate(m_pSvrDocTemplate);

	// 创建主 MDI 框架窗口
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// 仅当具有后缀时才调用 DragAcceptFiles
	//  在 MDI 应用程序中，这应在设置 m_pMainWnd 之后立即发生


	// 分析标准外壳命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
	cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	ParseCommandLine(cmdInfo);


	// 调度在命令行中指定的命令。如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// 主窗口已初始化，因此显示它并对其进行更新
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}



// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void CCommTestApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CCommTestApp 自定义加载/保存方法

void CCommTestApp::PreLoadState()
{
// 	BOOL bNameValid;
// 	CString strName;
// 	bNameValid = strName.LoadString(IDS_EDIT_MENU);
// 	ASSERT(bNameValid);
// 	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
// 	bNameValid = strName.LoadString(IDS_EXPLORER);
// 	ASSERT(bNameValid);
// 	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CCommTestApp::LoadCustomState()
{
}

void CCommTestApp::SaveCustomState()
{
}

// CCommTestApp 消息处理程序




int CCommTestApp::ExitInstance()
{
	// TODO: 在此添加专用代码和/或调用基类
	BCGCBProCleanUp();
	UnregisterLog( _T("MAIN"));

	return __super::ExitInstance();
}
