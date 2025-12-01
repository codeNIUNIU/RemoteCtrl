#pragma once
#include "pch.h"
#include "framework.h"



class CPacket {
public:
	CPacket():sHeader(0),sLength(0),sCmd(0),sSum(0) {}

	CPacket(const CPacket& packet) {
		sHeader = packet.sHeader;
		sLength = packet.sLength;
		sCmd = packet.sCmd;
		sData = packet.sData;
		sSum = packet.sSum;
	}

	CPacket &operator=(const CPacket& packet) {
		if (this == &packet) {
			return *this;
		}
		sHeader = packet.sHeader;
		sLength = packet.sLength;
		sCmd = packet.sCmd;
		sData = packet.sData;
		sSum = packet.sSum;
		return *this;
	}

	CPacket(const BYTE* pData, size_t& nSize) 
		: sHeader(0), sLength(0), sCmd(0), sSum(0) {
		size_t i = 0;
		for(; i < nSize; i++) {
			if (*(WORD*)pData[i + 1] == 0xFE) {
				sHeader = *(WORD*)pData[i + 1];
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {//包数据可能不全
			nSize = 0;
			return;
		}
		sLength = *(WORD*)pData[i + 1];
		i += 4;
		if((sLength +i) > nSize) {//包数据未完全接收
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)pData[i + 1];
		i += 2;
		if (sLength > 4) {
			sData.resize(sLength - 2 - 2);
			memcpy((void*)sData.c_str(), pData + i, sLength - 4);
			i += sLength - 4;
		}
		sSum = *(WORD*)pData[i + 1];
		i += 2;
		WORD sum = 0;
		for(size_t j = 0; j < sData.size(); j++) {
			sum += *(BYTE*)pData[i] & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;
			return;
		}
		nSize = 0;
	}

	~CPacket() {};

public:
	WORD sHeader;	//包头，固定为0xFE FF
	WORD sLength;	//包体长度（从sCmd开始到校验和结束）
	WORD sCmd;		//命令
	std::string sData;	//数据
	WORD sSum;		//校验和
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

#define MAX_PACKET_SIZE 4096
	int DealCommand() {
		if (m_client_sock == -1) {
			return -1;
		}
		char* recv_buf = new char[MAX_PACKET_SIZE];
		size_t index = 0;
		while (true) {
			size_t recv_len = recv(m_client_sock, recv_buf + index, MAX_PACKET_SIZE - index, 0);
			if (recv_len <= 0) {
				return -1;
			}
			index += recv_len;
			recv_len = index;
			m_packet = CPacket((BYTE*)recv_buf, recv_len);
			if (recv_len > 0) {
				memmove(recv_buf, recv_buf + recv_len, MAX_PACKET_SIZE - recv_len);
				return m_packet.sCmd;
			}
		}
		return -1;
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

