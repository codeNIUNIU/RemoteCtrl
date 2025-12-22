
// RemotClientDlg.h: 头文件
//

#pragma once
#include "ClientSocket.h"
#include "StatusDlg.h"

// 自定义消息ID
#define WM_SEND_PACKET (WM_USER + 1) //发送数据包消息

// CRemotClientDlg 对话框
class CRemotClientDlg : public CDialogEx
{
// 构造
public:
	CRemotClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTCLIENT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
	static void threadEntryForDownFile(void* arg);
	void threadDownFile();
	void LoadFileInfo();
	void LoadFileCurrent();
	CString CRemotClientDlg::GetPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	//1.获取磁盘分区
	//2.查看指定目录下的文件
	//3.打开文件
	//4.下载文件
	//5.删除文件
	//6.上传文件
	//7.锁机
	//8.解锁
	//9.删除文件
	//1981.连接测试
	//返回值是命令号，如果小于0 错误
	int SendCommandPacket(int sCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t nLength = 0);

// 实现
protected:
	HICON m_hIcon;
	CStatusDlg m_dlgStatus;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedWizfinish();
	afx_msg void OnBnClickedBtnTest();
	DWORD m_server_address;
	CString m_nPort;
	afx_msg void OnBnClickedButtonFileinfo();
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
	afx_msg LRESULT OnSendPacket(WPARAM wParam, LPARAM lParam);//定义自定义消息处理函数
};
