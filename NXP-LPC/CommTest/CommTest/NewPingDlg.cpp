// NewPingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CommTest.h"
#include "NewPingDlg.h"
#include "PingTest.h"


// CNewPingDlg 对话框

IMPLEMENT_DYNAMIC(CNewPingDlg, CBCGPDialog)

CNewPingDlg::CNewPingDlg(CWnd* pParent /*=NULL*/)
	: CBCGPDialog(CNewPingDlg::IDD, pParent)
	, m_nPingDataSize(32)
	, m_nIntTime(300)
	, m_nTaskCnt(1)
	, m_bAutoDelay(TRUE)
{

}

CNewPingDlg::~CNewPingDlg()
{
}

void CNewPingDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS_PING_DEST, m_ctlDestIp);
	DDX_Text(pDX, IDC_EDIT_PING_DATA_SIZE, m_nPingDataSize);
	DDV_MinMaxUInt(pDX, m_nPingDataSize, 32, 64000);
	DDX_Text(pDX, IDC_EDIT_PING_INT_TIME, m_nIntTime);
	DDV_MinMaxUInt(pDX, m_nIntTime, 200, 50000);
	DDX_Text(pDX, IDC_EDIT_PING_THREAD_CNT, m_nTaskCnt);
	DDV_MinMaxUInt(pDX, m_nTaskCnt, 1, MAX_PING_TASK);
	DDX_Check(pDX, IDC_CHECK_TIMEOUT_DELAY, m_bAutoDelay);
}


BEGIN_MESSAGE_MAP(CNewPingDlg, CBCGPDialog)
END_MESSAGE_MAP()


// CNewPingDlg 消息处理程序

BOOL CNewPingDlg::OnInitDialog()
{
	CBCGPDialog::OnInitDialog();
	 
	// TODO:  在此添加额外的初始化
	EnableVisualManagerStyle();
	
	DWORD dwIp = htonl( inet_addr("192.9.200.57"));
	m_ctlDestIp.SetAddress(dwIp);
	 
 

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CNewPingDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	UpdateData(TRUE);

	m_ctlDestIp.GetAddress(m_dwDestIp);


	CBCGPDialog::OnOK();
}
