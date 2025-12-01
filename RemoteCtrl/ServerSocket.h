#pragma once
#include "pch.h"
#include "framework.h"



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

	int DealCommand() {
		if (m_client_sock == -1) {
			return -1;
		}
		char recv_buf[1024] = "";
		while (true) {
			int recv_len = recv(m_client_sock, recv_buf, sizeof(recv_buf), 0);
			if (recv_len <= 0) {
				return -1;
			}
			//TODO: 处理命令

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

