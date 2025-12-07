#pragma once
#include "pch.h"
#include "framework.h"



#pragma pack(push)
#pragma pack(1)
//数据包格式和数据打包、解包
class CPacket{
public:
	CPacket():sHead(0), nLenth(0), sCmd(0), sSum(0){}

	//CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
	//	sHead = 0xFEFF;
	//	nLenth = nSize + 4;
	//	sCmd = nCmd;
	//	strData.resize(nSize);
	//	memcpy((void*)strData.c_str(), pData, nSize);
	//	sSum = 0;
	//	for (int j = 0;j < strData.size(); ++j) {
	//		sSum += BYTE(strData[j]) & 0xFF;
	//	}
	//}

	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLenth = nSize + 4; // 命令2 + 校验2
		sCmd = nCmd;
		if (nSize > 0) {
			strData.assign(reinterpret_cast<const char*>(pData), nSize);
		}else{
			strData.clear();
		}

		// 正确计算校验和
		sSum = 0;
		for (size_t j = 0; j < strData.size(); ++j) {
			sSum += static_cast<BYTE>(strData[j]);
		}
	}

	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLenth = pack.nLenth;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}

	CPacket(const BYTE* pData, size_t nSize) {
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
			if (i + 4 + 2 + 2 > nSize) {
				nSize = 0;
				return;
			}
			nLenth = *(DWORD*)(pData + i);
			i += 4;
			if(nLenth + i > nSize) {
				nSize = 0;
				return;
			}
			sCmd = *(DWORD*)(pData + i);
			i += 2;
			if (nLenth > 4) {
				strData.reserve(nLenth - 2 - 2);
				memcpy((void*)strData.c_str(), pData + i, nLenth - 2 - 2);
				i += nLenth - 2 - 2;
			}
			sSum = *(WORD*)(pData + i);
			i += 2;
			WORD sum = 0;
			for (size_t j = 0; j < strData.size(); j++) {
				sum += BYTE(strData[i]) & 0xFF;
			}
			if (sum == sSum) {
				nSize = i;
				return;
			}
			nSize = 0;
		}
	}

	CPacket& operator=(const CPacket& pack) {
		sHead = pack.sHead;
		nLenth = pack.nLenth;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
		return *this;
	}

	int Size() {
		return nLenth + 6;
	}

	//const char* Data() {
	//	strOut.resize(nLenth + 6);
	//	BYTE* pData = (BYTE*)strOut.c_str();
	//	*(WORD*)pData = sHead;
	//	pData += 2;
	//	*(DWORD*)(pData) = nLenth;
	//	pData += 4;
	//	*(WORD*)pData = sCmd;
	//	pData += 2;
	//	memcpy(pData, strData.c_str(), strData.size());
	//	pData += strData.size();
	//	*(WORD*)pData = sSum;
	//	return strData.c_str();
	//}

	const char* Data() {
		// 计算总包大小：包头2 + 长度4 + 命令2 + 数据N + 校验2
		const size_t totalSize = 2 + 4 + 2 + strData.size() + 2;
		strOut.resize(totalSize);

		BYTE* p = reinterpret_cast<BYTE*>(&strOut[0]);

		// 使用网络字节序（大端序）写入
		*p++ = static_cast<BYTE>(sHead >> 8);  // FE
		*p++ = static_cast<BYTE>(sHead & 0xFF); // FF

		// 写入长度（4字节大端序）
		*p++ = static_cast<BYTE>(nLenth >> 24);
		*p++ = static_cast<BYTE>(nLenth >> 16);
		*p++ = static_cast<BYTE>(nLenth >> 8);
		*p++ = static_cast<BYTE>(nLenth & 0xFF);

		// 写入命令（2字节大端序）
		*p++ = static_cast<BYTE>(sCmd >> 8);
		*p++ = static_cast<BYTE>(sCmd & 0xFF);

		// 写入数据
		memcpy(p, strData.data(), strData.size());
		p += strData.size();

		// 写入校验和（2字节大端序）
		*p++ = static_cast<BYTE>(sSum >> 8);
		*p++ = static_cast<BYTE>(sSum & 0xFF);

		return strOut.c_str();
	}


	~CPacket() {}

public:
	WORD sHead;				//包头，固定为FE FF
	WORD nLenth;			//包长度（从控制命令开始，到和校验结束）
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
		sockaddr_in cli_addr;
		int cli_addr_size = sizeof(cli_addr);
		m_client_sock = accept(m_sock, (sockaddr*)&cli_addr, &cli_addr_size);
		if (m_client_sock == -1) {
			return false;
		}
		return true;
	}

#define BUFFER_SIZE 4096

	int DealCommand() {
		if (m_client_sock == -1) {
			return -1;
		}

		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true) {
			size_t buffer_len = recv(m_client_sock, buffer + index, BUFFER_SIZE - index, 0);
			if (buffer_len <= 0) {
				return -1;
			}
			index += buffer_len;
			//TODO: 处理命令
			m_packet = CPacket((BYTE*)buffer, buffer_len);
			if (buffer_len > 0) {
				memmove(buffer, buffer + buffer_len, BUFFER_SIZE - buffer_len);
				index -= buffer_len;
				return m_packet.sCmd;
			}

		}
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

