#include "pch.h"
#include "ServerSocket.h"





CServerSocket* CServerSocket::m_instance = nullptr;
CServerSocket::CHelper CServerSocket::m_helper;
CServerSocket* pserver = CServerSocket::getInstance();

void Dump(BYTE* pData, size_t nSize)
{
    std::string strOut;
    for (size_t i = 0; i < nSize; i++) {
        char buf[8] = "";
        if (i > 0 && (i % 16 == 0)) {
            strOut += "\n";
        }
        snprintf(buf, sizeof(buf),"%02X ", pData[i] & 0xFF);
        strOut += buf;
    }
    strOut += "\n";

    OutputDebugStringA(strOut.c_str());
}
