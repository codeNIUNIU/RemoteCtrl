
// RemotClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemotClient.h"
#include "RemotClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemotClientDlg 对话框



CRemotClientDlg::CRemotClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTCLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemotClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
}

int CRemotClientDlg::SendCommandPacket(int sCmd, BYTE* pData, size_t nLength)
{
	UpdateData();//把数据从界面更新到全局变量m_server_address、m_nPort中
	CClientSocket* pClient = CClientSocket::getInstance();
	bool ret = pClient->InitSocket(m_server_address, atoi((LPCTSTR)m_nPort));
	if (!ret) {
		AfxMessageBox("网络初始化失败！");
		return -1;
	}
	CPacket pack(sCmd, pData, nLength);
	ret = pClient->SendData(pack);
	TRACE("send ret %d\r\n", ret);
	int cCmd = pClient->DealCommand();
	TRACE("ack: %d\r\n", cCmd);
	pClient->CloseSocket();
	return cCmd;
}

BEGIN_MESSAGE_MAP(CRemotClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemotClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BUTTON_FILEINFO, &CRemotClientDlg::OnBnClickedButtonFileinfo)
END_MESSAGE_MAP()


// CRemotClientDlg 消息处理程序

BOOL CRemotClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	m_server_address = 0x7F000001;
	m_nPort = _T("9527");
	UpdateData(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemotClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemotClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemotClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CRemotClientDlg::OnBnClickedBtnTest()
{
	SendCommandPacket(1981);
}


//void CRemotClientDlg::OnBnClickedButtonFileinfo()
//{
//	int ret = SendCommandPacket(1); //获取磁盘分区
//	if (ret == -1) {
//		AfxMessageBox(_T("命令处理失败！"));
//		return;
//	}
//	CClientSocket* pClient = CClientSocket::getInstance();
//	std::string drivers = pClient->GetPacket().strData;
//	std::string dr;
//	m_Tree.DeleteAllItems();
//	for (size_t i = 0;i < drivers.size();i++) {
//		TRACE("drivers[%d] = %c\r\n", i, drivers[i]);
//		if (drivers[i] == ',') {
//			dr += ':';
//			m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
//			dr.clear();
//			continue;
//		}
//		dr += drivers[i];
//	}
//}

void CRemotClientDlg::OnBnClickedButtonFileinfo()
{
	int ret = SendCommandPacket(1); //获取磁盘分区
	if (ret == -1) {
		AfxMessageBox(_T("命令处理失败！"));
		return;
	}
	CClientSocket* pClient = CClientSocket::getInstance();
	std::string drivers = pClient->GetPacket().strData;
	std::string dr;
	m_Tree.DeleteAllItems();
	for (size_t i = 0; i < drivers.size(); i++) {
		TRACE("drivers[%d] = %c\r\n", i, drivers[i]);
		if (drivers[i] == ',') {
			dr += ':';
			m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
	// 处理最后一个分区（如果有）
	if (!dr.empty()) {
		dr += ':';
		m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
	}
}
