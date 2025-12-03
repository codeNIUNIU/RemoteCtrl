#pragma once
#include "pch.h"
#include "framework.h"



class CPacket{
public:
	CPacket():sHead(0),sSum(0),nLenth(0),sCmd(0){}

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

	~CPacket() {}

public:
	WORD sHead;				//包头，固定为FE FF
	DWORD nLenth;			//包长度（从控制命令开始，到和校验结束）
	DWORD sCmd;				//控制命令
	std::string strData;	//包数据
	WORD sSum;				//和校验
};

class CServerSocket
{
public:
	static CServerSocket* GetInstance() {
        if (m_instance == nullptr) {
			m_instance = new CServerSocket();
        }
        return m_instance;
	}

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

	class CHelper {
	public:
		CHelper() {
			CServerSocket::GetInstance();
		}
        ~CHelper() {
			CServerSocket::realseInstance();
		}
	};
    static CHelper m_helper; 
};

