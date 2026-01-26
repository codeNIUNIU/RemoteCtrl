// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemotClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include "RemotClientDlg.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{
	m_nObjWidth = -1;
	m_nObjHeight = -1;
}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
	ON_BN_CLICKED(IDC_BIT_LOCK, &CWatchDialog::OnBnClickedBitLock)
	ON_BN_CLICKED(IDC_BIT_UNLOCK, &CWatchDialog::OnBnClickedBitUnlock)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point,bool isScreen)
{
	CRect clientRect;
	if (isScreen) {//如果是全局坐标
		ScreenToClient(&point);//将用户点转换为客户端点,全局坐标转换为客户区域坐标
	}
	m_picture.GetWindowRect(&clientRect);//获取图片控件的客户区域矩形

	return CPoint(point.x * m_nObjWidth / clientRect.Width(), point.y * m_nObjHeight / clientRect.Height());//将用户点转换为远程屏幕点
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetTimer(0, 45, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0) {
		CRemotClientDlg* pParent = (CRemotClientDlg*)GetParent();
		if (pParent->isFull()){
			CRect rect;
			m_picture.GetWindowRect(&rect);
			// pParent->GetImage().BitBlt(m_picture.GetDC(), 0, 0, SRCCOPY);
			if (m_nObjWidth == -1) {
				m_nObjWidth = pParent->GetImage().GetWidth();
			}
			if (m_nObjHeight == -1) {
				m_nObjHeight = pParent->GetImage().GetHeight();
			}
			pParent->GetImage().StretchBlt(//拉伸图像到指定区域
				m_picture.GetDC()->GetSafeHdc(), 0, 0,
				rect.Width(), rect.Height(), SRCCOPY);
			m_picture.Invalidate();
			pParent->GetImage().Destroy();
			pParent->SetImageStatus();
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nObjHeight != -1 && m_nObjWidth != -1) {
		//坐标转换，将用户点转换为远程屏幕点
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 2;//双击
		CClientSocket* pClient = CClientSocket::getInstance();
		CRemotClientDlg* pParent = (CRemotClientDlg*)GetParent();
		pParent->SendMessage((WM_SEND_PACKET), 5 << 1 | 1, (WPARAM)&event);
	}

	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_nObjHeight != -1 && m_nObjWidth != -1) {
		TRACE("x = %d    y = %d\r\n", point.x, point.y);
		//坐标转换，将用户点转换为远程屏幕点
		CPoint remote = UserPoint2RemoteScreenPoint(point, true);
		TRACE("x = %d    y = %d\r\n", point.x, point.y);
		//封装
		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 2;//按下
		CClientSocket* pClient = CClientSocket::getInstance();
		CRemotClientDlg* pParent = (CRemotClientDlg*)GetParent();
		pParent->SendMessage((WM_SEND_PACKET), 5 << 1 | 1, (WPARAM)&event);
	}

	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nObjHeight != -1 && m_nObjWidth != -1) {
		//坐标转换，将用户点转换为远程屏幕点
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 3;//弹起
		CClientSocket* pClient = CClientSocket::getInstance();
		CRemotClientDlg* pParent = (CRemotClientDlg*)GetParent();
		pParent->SendMessage((WM_SEND_PACKET), 5 << 1 | 1, (WPARAM)&event);
	}

	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nObjHeight != -1 && m_nObjWidth != -1) {
		//坐标转换，将用户点转换为远程屏幕点
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 1;//双击
		CClientSocket* pClient = CClientSocket::getInstance();
		CRemotClientDlg* pParent = (CRemotClientDlg*)GetParent();
		pParent->SendMessage((WM_SEND_PACKET), 5 << 1 | 1, (WPARAM)&event);
	}

	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_nObjHeight != -1 && m_nObjWidth != -1) {
		//坐标转换，将用户点转换为远程屏幕点
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 2;//按下
		CClientSocket* pClient = CClientSocket::getInstance();
		CRemotClientDlg* pParent = (CRemotClientDlg*)GetParent();
		pParent->SendMessage((WM_SEND_PACKET), 5 << 1 | 1, (WPARAM)&event);
	}

	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_nObjHeight != -1 && m_nObjWidth != -1) {
		//坐标转换，将用户点转换为远程屏幕点
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 3;//弹起
		CClientSocket* pClient = CClientSocket::getInstance();
		CRemotClientDlg* pParent = (CRemotClientDlg*)GetParent();
		pParent->SendMessage((WM_SEND_PACKET), 5 << 1 | 1, (WPARAM)&event);
	}

	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_nObjHeight != -1 && m_nObjWidth != -1) {
		//坐标转换，将用户点转换为远程屏幕点
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 8;//没有按键
		event.nAction = 0;//移动
		CClientSocket* pClient = CClientSocket::getInstance();
		CRemotClientDlg* pParent = (CRemotClientDlg*)GetParent();
		pParent->SendMessage((WM_SEND_PACKET), 5 << 1 | 1, (WPARAM)&event);
	}

	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()
{
	if (m_nObjHeight != -1 && m_nObjWidth != -1) {
		CPoint point;
		GetCursorPos(&point);
		//坐标转换，将用户点转换为远程屏幕点
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 0;//0表示左键
		event.nAction = 0;//单击
		CClientSocket* pClient = CClientSocket::getInstance();
		CRemotClientDlg* pParent = (CRemotClientDlg*)GetParent();
		pParent->SendMessage((WM_SEND_PACKET), 5 << 1 | 1, (WPARAM)&event);
	}
}


void CWatchDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}




void CWatchDialog::OnBnClickedBitLock()
{
	// TODO: 在此添加控件通知处理程序代码
	CRemotClientDlg* pParent = (CRemotClientDlg*)GetParent();
	pParent->SendMessage((WM_SEND_PACKET), 7 << 1 | 1);
}


void CWatchDialog::OnBnClickedBitUnlock()
{
	// TODO: 在此添加控件通知处理程序代码
	CRemotClientDlg* pParent = (CRemotClientDlg*)GetParent();
	pParent->SendMessage((WM_SEND_PACKET), 8 << 1 | 1);
}
