#pragma once
#include "pch.h"
#include "framework.h"



#pragma pack(push)
#pragma pack(1)
//数据包格式和数据打包、解包
class CPacket{
public:
	CPacket():sHead(0), nLength(0), sCmd(0), sSum(0){}

	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
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

	CPacket(const BYTE* pData, size_t& nSize) {
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
			TRACE("i = %d 包数据不全！\r\n", i);
			return;
		}
		nLength = *(DWORD*)(pData + i); 
		i += 4;
		if (nLength + i > nSize) { //包未完全接收到，就返回，解析失败
			nSize = 0;
			TRACE("包未完全接收到！\r\n");
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
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

	const char* Data() {
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
}MOUSEEVENT,*PMOUSEEVENT;

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


//网络服务类，单例模式
class CServerSocket
{
public:
	static CServerSocket* getInstance() {
        if (m_instance == nullptr) {
			m_instance = new CServerSocket();
        }
        return m_instance;
	}

	//初始化网络服务
	bool InitSocket() {
		if (m_sock == -1) {
			return false;
		}
		sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(9527);
		//绑定 bind
		if (bind(m_sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
			return false;
		}
		//监听 listen
		if (listen(m_sock, 1) == -1) {
			return false;
		}

		return true;
	}

	bool AcceptClient() {
		TRACE("Enter AcceptClient!\r\n");
		sockaddr_in cli_addr;
		int cli_addr_size = sizeof(cli_addr);
		m_client_sock = accept(m_sock, (sockaddr*)&cli_addr, &cli_addr_size);
		TRACE("m_client_sock = %d\r\n",m_client_sock);
		if (m_client_sock == -1) {
			return false;
		}
		return true;
	}

#define BUFFER_SIZE 4096
	int DealCommand() {
		if (m_client_sock == -1) return -1;
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == NULL) {
			TRACE("内存不足!\r\n");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true) {
			size_t len = recv(m_client_sock, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0) {
				delete[]buffer;
				return -1;
			}
			TRACE("recv len = %d\r\n", len);
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				delete[]buffer;
				return m_packet.sCmd;
			}
		}
		delete[]buffer;
		return -1;
	}

	bool SendData(const char* pData, size_t nSize) {
		if (m_client_sock == -1) {
			return false;
		}
		return send(m_client_sock, pData, nSize, 0) > 0;
	}

	bool SendData(CPacket& pack) {
		if (m_client_sock == -1) {
			return false;
		}
		return send(m_client_sock, pack.Data(), pack.Size(), 0) > 0;
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

	void CloseClient()
	{
		closesocket(m_client_sock);
		m_client_sock = INVALID_SOCKET;
	}

private:
	static CServerSocket* m_instance;
	SOCKET m_sock;
	SOCKET m_client_sock;
	CPacket m_packet;

	CServerSocket& operator=(const CServerSocket& ss){}

    CServerSocket(const CServerSocket& ss){
		m_sock = ss.m_sock;
		m_client_sock = ss.m_client_sock;
	}

	CServerSocket(){
		m_client_sock = INVALID_SOCKET;
		if (!InitSockEnv()) {
			MessageBox(NULL,_T("初始化套接字环境失败，请检查网络设置"), _T("初始化失败"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}

	~CServerSocket(){
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
			CServerSocket* temp = m_instance;
			m_instance = nullptr;
            delete temp;
		}
	}

	//辅助类，用于构建网络服务类与释放网络服务类资源
	class CHelper {
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
        ~CHelper() {
			CServerSocket::realseInstance();
		}
	};
    static CHelper m_helper; 
};

