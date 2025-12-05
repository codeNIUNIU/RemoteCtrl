// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "direct.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

void Dump(BYTE* pData, size_t nSize) {
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

//磁盘分区信息
int MakeDriverInfo() { //1->A: 2->B: 3->C: ...Win系统盘符从1开始，共26个盘符
    std::string result;
    for(int i = 1; i < 27; i++) {
        if (_chdrive(i) == 0) { //切换到指定驱动器，如果能切换成功，说明驱动存在
            if (result.size() > 0) {
                result += ',';
            }
            result += 'A' + i - 1;
        }
	}

    CPacket pack(1, (BYTE*)result.c_str(), result.size());
    Dump((BYTE*)&pack, pack.nLenth + 6);

    return 0;
    //CServerSocket::getInstance()->SendData(CPacket(1, (BYTE*)result.c_str(), result.size()));
    //return NULL;
}

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
      //      // TODO: 在此处为应用程序的行为编写代码。
      //      CServerSocket* pserver = CServerSocket::GetInstance();
      //      if (pserver->InitSocket() == false) {
      //          MessageBox(NULL, _T("网络初始化异常，请检查网络!"), _T("网络初始化失败!"), MB_OK | MB_ICONERROR);
      //          exit(0);
      //      }
      //      int count = 0;
      //      while (CServerSocket::GetInstance() != NULL) {
      //          if (pserver->AcceptClient() == false) {
      //              if (count >= 3) {
      //                  MessageBox(NULL, _T("多次无法正常接入用户，结束程序!"), _T("接入用户失败!"), MB_OK | MB_ICONERROR);
	  //					//exit(0);
      //              }
      //              MessageBox(NULL, _T("无法正常接入用户，正在重试!"), _T("接入用户失败!"), MB_OK | MB_ICONERROR);
      //              count++;
      //          }
      // 
      //          int ret = pserver->DealCommand();
      //          //TODO: 处理命令
      //      }

            //文件需求 - 观察、打开、下载、删除

            //测试MakeDriverInfo函数
            MakeDriverInfo();


        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
