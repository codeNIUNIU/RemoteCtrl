
// RemotClientDlg.h: 头文件
//

#pragma once
#include "ClientSocket.h"

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
	int SendCommandPacket(int sCmd, BYTE* pData = NULL, size_t nLength = 0);

// 实现
protected:
	HICON m_hIcon;

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
};
