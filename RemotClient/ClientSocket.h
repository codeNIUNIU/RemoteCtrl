#pragma once
#include <string>
#include "pch.h"
#include "framework.h"
#include <vector>



void Dump(BYTE* pData, size_t nSize);
#pragma pack(push)
#pragma pack(1)
//数据包格式和数据打包、解包
class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}

	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) //打包数据
	{
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}

	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}

	CPacket(const BYTE* pData, size_t& nSize) //解析包数据
	{
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2; //why? 防止一种特殊情况
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {//包数据可能不全，或者包头未能全部接收到
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) { //包未完全接收到，就返回，解析失败
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			// TRACE("rece:[%s]\r\n", strData.c_str() + 12);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i; //head length data
			return;
		}
		nSize = 0;
	}

	CPacket& operator=(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
		return *this;
	}

	int Size() {
		return nLength + 6;
	}

	const char* Data() //组包，返回可以发送的数据
	{
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;
		pData += 2;
		*(DWORD*)(pData) = nLength;
		pData += 4;
		*(WORD*)pData = sCmd;
		pData += 2;
		memcpy(pData, strData.c_str(), strData.size());
		pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

	~CPacket() {}

public:
	WORD sHead;				//包头，固定为FE FF
	DWORD nLength;			//包长度（从控制命令开始，到和校验结束）
	WORD sCmd;				//控制命令
	std::string strData;	//包数据
	WORD sSum;				//和校验
	std::string strOut;		//整个包的数据
};
#pragma pack(pop)

//鼠标事件数据结构
typedef struct MouseEvent {
	MouseEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击、移动、双击
	WORD nButton;//左键、右键、中键 0\2\1
	POINT ptXY;//坐标
}MOUSEEVENT, * PMOUSEEVENT;

typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;//是否有效
	BOOL IsDirectory;//是否为目录 0 否 1 是
	BOOL HasNext;//是否还有后续文件 0 没有 1 有
	char szFileName[256];//文件名
}FILEINFO, * PFILEINFO;


std::string GetErrorInfo(int wsaErrCode); //函数声明，在.cpp文件实现

//网络服务类，单例模式
class CClientSocket
{
public:
	static CClientSocket* getInstance() {
		if (m_instance == nullptr) {
			m_instance = new CClientSocket();
		}
		return m_instance;
	}

	//初始化网络服务
	bool InitSocket(int nIP,int nPort) {
		TRACE("Init Client Socket!\r\n");
		if (m_sock != INVALID_SOCKET) {
			CloseSocket();
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sock == -1) {
			return false;
		}
		sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		//TRACE("addr %08x  nIP %08x\r\n", inet_addr("127.0.0.1"), nIP);
		TRACE("addr %08x  nIP %08x\r\n", inet_addr("10.0.2.15"), nIP);
		serv_addr.sin_addr.s_addr = htonl(nIP);
		serv_addr.sin_port = htons(nPort);
		if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
			AfxMessageBox("IP地址无效");
            return false;
		}
		//连接
		int ret = connect(m_sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
		if (ret == -1) {
			AfxMessageBox("连接失败");
			TRACE("连接失败：%d %s \r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
			return false;
        }
		return true;
	}

#define BUFFER_SIZE 4096000

	int DealCommand() {
		// TRACE("m_sock = %d\r\n", m_sock);
		if (m_sock == -1) {
			return -1;
		}
		char* buffer = m_buffer.data();
		static size_t index = 0;
		while (true) {
			size_t buffer_len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);
			if ((buffer_len <= 0) && (index <= 0)) {
				TRACE("recv error: %d %s \r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
				return -1;
			}
			// TRACE("rece len = %d(0x%08X)  index = %d(0x%08X)\r\n", len, len, index, index);
			// Dump((BYTE*)buffer, index);
			index += buffer_len;
			buffer_len = index;
			m_packet = CPacket((BYTE*)buffer, buffer_len);
			if (buffer_len > 0) {
				memmove(buffer, buffer + buffer_len, index - buffer_len);
				index -= buffer_len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	int DealCommand1() {
		if (m_sock == -1) return -1;
		char* buffer = m_buffer.data();//TODO:多线程发送命令时可能会出现冲突
		static size_t index = 0; //之前为非静态
		while (true) {
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);
			if ((len <= 0) && (index <= 0)) {
				return -1;
			}
			// TRACE("rece len = %d(0x%08X)  index = %d(0x%08X)\r\n", len, len, index, index);
			Dump((BYTE*)buffer, index);
			index += len;
			len = index;  
			// TRACE("rece len = %d(0x%08X)  index = %d(0x%08X)\r\n", len, len, index, index);
			m_packet = CPacket((BYTE*)buffer, len);
			// TRACE("command %d\r\n", m_packet.sCmd);
			if (len > 0) { //为什么会跳过啊?
				memmove(buffer, buffer + len, index - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	bool SendData(const char* pData, size_t nSize) {
		if (m_sock == -1) {
			return false;
		}
		return send(m_sock, pData, nSize, 0) > 0;
	}

	bool SendData(CPacket& pack) {
		TRACE("m_sock = %d\r\n", m_sock);
		if (m_sock == -1) {
			return false;
		}

		int ret = send(m_sock, pack.Data(), pack.Size(), 0);
		return ret > 0;
	}

	bool GetFilePath(std::string& strPath)
	{
		if ((m_packet.sCmd == 2) || (m_packet.sCmd == 3) || (m_packet.sCmd == 4)) {//判断当前命令是否为处理文件的命令
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}

	bool GetMouseEvent(MOUSEEVENT& mouse)
	{
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEVENT));
			return true;
		}
		return false;
	}

	CPacket& GetPacket()
	{
		return m_packet;
	}

	void CloseSocket() {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}

private:
	std::vector<char> m_buffer;
	static CClientSocket* m_instance;
	SOCKET m_sock;
	CPacket m_packet;

	CClientSocket& operator=(const CClientSocket& ss) {}

	CClientSocket(const CClientSocket& ss) {
		m_sock = ss.m_sock;
	}

	CClientSocket() {
		m_sock = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("初始化套接字环境失败，请检查网络设置!"), _T("网络初始化失败!"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_buffer.resize(BUFFER_SIZE);
		memset(m_buffer.data(), 0, BUFFER_SIZE);
	}

	~CClientSocket() {
		WSACleanup();
		if (m_sock != -1) {
			closesocket(m_sock);
		}
	}

	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}

	static void realseInstance() {
		if (m_instance != nullptr) {
			CClientSocket* temp = m_instance;
			m_instance = nullptr;
			delete temp;
		}
	}

	//辅助类，用于构建网络服务类与释放网络服务类资源
	class CHelper {
	public:
		CHelper() {
			CClientSocket::getInstance();
		}
		~CHelper() {
			CClientSocket::realseInstance();
		}
	};
	static CHelper m_helper;
};




