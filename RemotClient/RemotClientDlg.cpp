
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
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

int CRemotClientDlg::SendCommandPacket(int sCmd, bool bAutoClose, BYTE* pData, size_t nLength)
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
	if (bAutoClose) {
		pClient->CloseSocket();
	}

	return cCmd;
}

BEGIN_MESSAGE_MAP(CRemotClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemotClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BUTTON_FILEINFO, &CRemotClientDlg::OnBnClickedButtonFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemotClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemotClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemotClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemotClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemotClientDlg::OnDeleteFile)
	ON_COMMAND(ID_RUN_FILE, &CRemotClientDlg::OnRunFile)
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
			HTREEITEM hTemp = m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
			m_Tree.InsertItem(NULL, hTemp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
	// 处理最后一个分区（如果有）
	if (!dr.empty()) {
		dr += ':';
		HTREEITEM hTemp = m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
		m_Tree.InsertItem("", hTemp, TVI_LAST);
	}
}

void CRemotClientDlg::LoadFileInfo()
{
	CPoint pMouse;
	GetCursorPos(&pMouse);
	m_Tree.ScreenToClient(&pMouse);//将鼠标坐标转换为树形控件的客户端坐标
	HTREEITEM hTreeSelected = m_Tree.HitTest(pMouse, 0);//获取点击到的节点
	if (hTreeSelected == NULL) {//判断是否点击到了某个节点
		return;
	}
	if (m_Tree.GetChildItem(hTreeSelected) == NULL) {//如果是文件则直接返回
		return;
	}
	DeleteTreeChildrenItem(hTreeSelected);//删除选中节点的所有子节点
	m_List.DeleteAllItems();//清空列表控件
	CString strPath = GetPath(hTreeSelected);//获取选中节点的完整路径
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());

	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	CClientSocket* pClient = CClientSocket::getInstance();
	int rcount = 0;
	while (pInfo->HasNext) {
		TRACE("[%s] isdir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (pInfo->IsDirectory) {
			if (CString(pInfo->szFileName) == "." || CString(pInfo->szFileName) == "..") {
				int cmd = pClient->DealCommand();
				TRACE("ack: %d\r\n", cmd);
				if (cmd < 0) {
					break;
				}
				pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
			HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
			m_Tree.InsertItem("", hTemp, TVI_LAST);
		}
		else {
			m_List.InsertItem(0, pInfo->szFileName);//文件不显示在树上，直接在右边显示
		}

		int cmd = pClient->DealCommand();
		// TRACE("ack: %d\r\n", cmd);
		if (cmd < 0) {
			break;
		}
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
		rcount++;
	}
	pClient->CloseSocket();
	TRACE("rcount = %d\r\n", rcount);
}

void CRemotClientDlg::LoadFileCurrent()
{
	HTREEITEM hTree = m_Tree.GetSelectedItem();
	if (hTree == NULL) {//判断是否点击到了某个节点
		return;
	}
	CString strPath = GetPath(hTree);//获取选中节点的完整路径

	m_List.DeleteAllItems();//清空列表控件
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());

	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	CClientSocket* pClient = CClientSocket::getInstance();
	while (pInfo->HasNext) {
		// TRACE("pInfo->szFileName = %s\r\n", pInfo->szFileName);
		if (!pInfo->IsDirectory) {//只需要更新文件列表
			m_List.InsertItem(0, pInfo->szFileName);//文件不显示在树上，直接在右边显示
		}

		int cmd = pClient->DealCommand();
		TRACE("ack: %d\r\n", cmd);
		if (cmd < 0) {
			break;
		}
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}
	pClient->CloseSocket();

}

//用于获取树形控件中指定节点的完整路径
//如果树形结构是 C: -> Windows -> System32，双击 System32 节点，函数会返回 C:\Windows\System32\
//
CString CRemotClientDlg::GetPath(HTREEITEM hTree)
{
	CString strRet, strTemp;
	do {
		strTemp = m_Tree.GetItemText(hTree);
		strRet = strTemp + "\\" + strRet;
		hTree = m_Tree.GetParentItem(hTree);
	} while (hTree != NULL);
	return strRet;
}

//删除树形控件中指定节点的所有直接子节点
//用于在刷新目录列表时，避免重复显示已存在的子目录
void CRemotClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do
	{
		hSub = m_Tree.GetChildItem(hTree);
		if (hSub != NULL) {
			m_Tree.DeleteItem(hSub);
		}
	} while (hSub != NULL);
}

//处理树形控件的双击事件，用于查看指定目录下的文件列表
void CRemotClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	LoadFileInfo();
}

//处理树形控件的单击事件，用于刷新指定目录下的文件列表
void CRemotClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	LoadFileInfo();
}

//处理列表控件的右键点击事件，用于弹出文件操作菜单
void CRemotClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	CPoint ptMouse,ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse; //一开始没给ptList 传值
	m_List.ScreenToClient(&ptList);
	int ListSelected = m_List.HitTest(ptList);
	if (ListSelected < 0) {
		return; //如果没点中任何项
	}
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);  //加载整个菜单资源
	CMenu* pPupup = menu.GetSubMenu(0);   
	if (pPupup != NULL) {
		pPupup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this); //弹出
	}
}

//处理文件操作菜单中的下载文件选项
void CRemotClientDlg::OnDownloadFile()
{
	int nListSelected = m_List.GetSelectionMark();//获取选中的文件项
	CString strFile = m_List.GetItemText(nListSelected, 0);//获取选中文件项的文件名
	//将选中的文件项的文件名添加到下载路径中
	CFileDialog dlg(FALSE,NULL, strFile,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		NULL, this);
	if (dlg.DoModal() == IDOK) {
		FILE* pFile = fopen(dlg.GetPathName(), "wb+");
		if (pFile == NULL) {
			AfxMessageBox(_T("本地文件创建失败或没有权限!!!"));
			return;
		}

		HTREEITEM hSelected = m_Tree.GetSelectedItem();//获取选中的目录项
		strFile = GetPath(hSelected) + strFile;//将选中的文件项的文件名添加到选中的目录项的完整路径中
		TRACE("strFile = %s\r\n", LPCSTR(strFile));
		CClientSocket* pClient = CClientSocket::getInstance();
		do
		{
			int ret = SendCommandPacket(4, false, (BYTE *)(LPCTSTR)strFile, strFile.GetLength());
			if (ret < 0){
				AfxMessageBox(_T("下载文件失败"));
				TRACE("下载文件失败，错误码：%d\r\n", ret);
				break;
			}

			long long nLehgth = *((long long *)pClient->GetPacket().strData.c_str());
			if (nLehgth == 0){
				AfxMessageBox(_T("文件长度为0或无法下载"));
				break;
			}

			long long nCount = 0;
			while (nCount < nLehgth){
				int ret = pClient->DealCommand();
				if (ret < 0){
					AfxMessageBox(_T("传输失败"));
					TRACE("传输失败\r\n");
					break;
				}
				fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
				nCount += pClient->GetPacket().strData.size();
			}
		} while (false);
		fclose(pFile);
		pClient->CloseSocket();
	}
}

//处理文件操作菜单中的删除文件选项
void CRemotClientDlg::OnDeleteFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();//获取选中的目录项
	CString strPath = GetPath(hSelected);			//获取选中目录项的完整路径
	int nSelected = m_List.GetSelectionMark();		//获取选中的文件项
	CString strFile = m_List.GetItemText(nSelected, 0);//获取选中文件项的文件名
	strFile = strPath + strFile;						//将目录路径和文件名拼接起来
	int ret = SendCommandPacket(9, true, (BYTE *)(LPCSTR)strFile, strFile.GetLength());//发送删除文件命令
	if (ret < 0){
		AfxMessageBox(_T("删除文件失败"));
	}
	
	LoadFileCurrent();//刷新目录列表
}

//处理文件操作菜单中的运行文件选项
void CRemotClientDlg::OnRunFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();//获取选中的目录项
	CString strPath = GetPath(hSelected);			//获取选中目录项的完整路径
	int nSelected = m_List.GetSelectionMark();		//获取选中的文件项
	CString strFile = m_List.GetItemText(nSelected, 0);//获取选中文件项的文件名
	strFile = strPath + strFile;						//将目录路径和文件名拼接起来
	int ret = SendCommandPacket(3, true, (BYTE *)(LPCSTR)strFile, strFile.GetLength());//发送打开文件命令
	if (ret < 0){
		AfxMessageBox(_T("打开文件失败"));
	}
}
