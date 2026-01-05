// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "direct.h"
#include <io.h>
#include <list>
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;


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

    CPacket pack(1, (BYTE*)result.c_str(), result.size());//打包用的
    Dump((BYTE*)pack.Data(), pack.Size());
    CServerSocket::getInstance()->SendData(pack);

    return 0;
}

//查看指定目录下的文件
int MakeDirectoryInfo()
{
   std::string strPath;
   //std::list<FILEINFO> IsFileInfos;

   if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
       OutputDebugString(_T("当前的命令，不是获取文件列表，命令解析错误！！！"));
       return -1;
   }
   if (_chdir(strPath.c_str()) != 0) {
       FILEINFO finfo;
       finfo.HasNext = FALSE;
       CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
       CServerSocket::getInstance()->SendData(pack);
       OutputDebugString(_T("没有权限访问该目录！！"));

       return -2;
   }
   
   _finddata_t fdata;
   int hfind = 0;
   if ((hfind = _findfirst("*", &fdata)) == -1) {
       OutputDebugString(_T("没有找到任何文件！！"));
       FILEINFO finfo;
       finfo.HasNext = FALSE;
       CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
       CServerSocket::getInstance()->SendData(pack);
       return -3;
   }
   int scount = 0;
   do {
       FILEINFO finfo;
       finfo.IsInvalid = FALSE;
       finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
       memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
       TRACE("finfo.szFileName = %s\r\n", finfo.szFileName);
       CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
       bool sendResult = CServerSocket::getInstance()->SendData(pack);//发送信息到控制端
       scount++;
   } while (!_findnext(hfind,&fdata));
    TRACE("scount = %d\r\n", scount);

   //最后发送完成后通知客户端
   FILEINFO finfo;
   finfo.HasNext = FALSE;
   CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
//    TRACE("MakeDirectoryInfo: Sending final packet, HasNext=FALSE\r\n");
   CServerSocket::getInstance()->SendData(pack);

   return 0;
}

//运行文件
int RunFile()
{
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);//打开文件

    CPacket pack(3, NULL, 0);
    CServerSocket::getInstance()->SendData(pack);

    return 0;
}

//下载文件，把文件从服务端发送到控制端
int DownloadFile()
{
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    long long data = 0;
    FILE* pFile = NULL;
    errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
        
    if (err != 0 || pFile == NULL) {
        CPacket pack(4, (BYTE*)&data, 8);
        CServerSocket::getInstance()->SendData(pack);
        return -1;
    }
    if (pFile != NULL) {
        fseek(pFile, 0, SEEK_END);
        data = _ftelli64(pFile);
        CPacket head(4, (BYTE*)&data, 8);
        CServerSocket::getInstance()->SendData(head);

        fseek(pFile, 0, SEEK_SET);
        char buffer[1024] = "";
        size_t rlen = 0;
        do {
            rlen = fread(buffer, 1, 1024, pFile);
            CPacket pack(4, (BYTE*)buffer, rlen);
            CServerSocket::getInstance()->SendData(pack);
        } while (rlen >= 1024);

        fclose(pFile);
    }

    CPacket pack(4, NULL, 0);
    CServerSocket::getInstance()->SendData(pack);

    return 0;
}

//鼠标操作
int MouseEvent()
{
    MOUSEEVENT mouse;
    if (CServerSocket::getInstance()->GetMouseEvent(mouse)) {
        DWORD nFlags = 0;
        switch (mouse.nButton) {
        case 0://左键
            nFlags = 1;
            break;
        case 1://右键
            nFlags = 2;
            break;
        case 2://中键
            nFlags = 4;
            break;
        case 8://没有按键
            nFlags = 8;
            break;
        }

        if (nFlags != 8) {
            SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);//设置鼠标位置
        }

        switch (mouse.nAction) {
        case 0://单击
            nFlags |= 0X10;
            break;
        case 1://双击
            nFlags |= 0X20;
            break;
        case 3://点击
            nFlags |= 0X40;
            break;
        case 4://放开
            nFlags |= 0X80;
            break;
        default:
            break;
        }
        TRACE("mouse event: %08X x: %d  y: %d\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);

        switch (nFlags) {
        case 0x21://左键双击
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());//系统API
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x11://左键单击
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());//系统API
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x41://左键按下
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81://左键放开
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x22://右键双击
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());//系统API
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x12://右键单击
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());//系统API
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x42://右键按下
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82://右键放开
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x24://中键双击
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());//系统API
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x14://中键单击
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());//系统API
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x44://中键按下
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x84://中键放开
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x08://单纯的鼠标移动
            mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
            break;
        }

        CPacket pack(4, NULL, 0);
        CServerSocket::getInstance()->SendData(pack);
    }
    else {
        OutputDebugString(_T("获取鼠标操作参数失败！！"));
        return -1;
    }

    return 0;
}

//屏幕监控
int SendScreen()
{
    CImage screen;
    HDC hScreen = ::GetDC(NULL);//获取设备上下文
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);//返回值是24位RGB888   ARGB888是32位，多了透明度
    int nWidth = GetDeviceCaps(hScreen, HORZRES);
    int nHeight = GetDeviceCaps(hScreen, VERTRES);
    screen.Create(nWidth, nHeight, nBitPerPixel);//创建图像
    BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
    ReleaseDC(NULL, hScreen);

    //保存到内存中
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
    if (hMem == NULL) {
        return -1;
    }
    IStream* pStream = NULL;//创建流对象
    HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//创建流对象，将内存映射到流中
    if (ret == S_OK) {
        screen.Save(pStream, Gdiplus::ImageFormatPNG);//将图像保存到流中
        LARGE_INTEGER bg = { 0 };//设置流指针位置为0
        pStream->Seek(bg, STREAM_SEEK_SET, NULL);//将流指针移动到流的开始位置
        PBYTE pData = (PBYTE)GlobalLock(hMem);//将内存映射到指针中
        SIZE_T nSize = GlobalSize(hMem);//获取内存映射的大小
        CPacket pack(6, pData, nSize);//创建数据包，命令码为6，数据为指针pData，数据大小为nSize
        CServerSocket::getInstance()->SendData(pack);//发送数据包
        GlobalUnlock(hMem);//解锁内存映射
        
    }

    //screen.Save(pStream, Gdiplus::ImageFormatPNG);//sava方法的重载，可以将图片保存在数据流中

    //保存为图片文件
    //DWORD tick = GetTickCount64();
    //screen.Save(_T("test2020.png"), Gdiplus::ImageFormatPNG);
    //TRACE("png: %d\r\n", GetTickCount64() - tick);
    //tick = GetTickCount64();
    //screen.Save(_T("test2020.jpg"), Gdiplus::ImageFormatJPEG);
    //TRACE("jpg: %d\r\n", GetTickCount64() - tick);

    pStream->Release();
    GlobalFree(hMem);
    screen.ReleaseDC();

    return 0;
}

#include "LockInfoDialog.h"
#include "resource.h"
CLockInfoDialog dlg;
unsigned threadid = 0;

//创建子线程用于锁机
unsigned _stdcall threadLockDlg(void* arg)
{
    TRACE("%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
    // 创建并显示遮罩窗口
    dlg.Create(IDD_DIALOG_INFO, NULL);
    dlg.ShowWindow(SW_SHOW);
    //遮蔽后台窗口,设置全屏遮罩
    CRect rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = GetSystemMetrics(SM_CXFULLSCREEN);//获取屏幕尺寸并设置窗口为全屏
    rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
    rect.bottom *= LONG(rect.bottom * 1.03);//增加高度，确保完全覆盖任务栏
    TRACE("right = %d bottom = %d\r\n", rect.right, rect.bottom);
    dlg.MoveWindow(rect);
    //窗口置顶,将窗口设置为最顶层，防止其他窗口覆盖
    //dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    //限制鼠标功能
    ShowCursor(false);// 隐藏鼠标指针
    //隐藏任务栏
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
    //限制鼠标活动范围
    //dlg.GetWindowRect(rect);
    rect.left = 0;
    rect.top = 0;
    rect.right = 1;
    rect.bottom = 1;
    ClipCursor(rect);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {//监听事件
        TranslateMessage(&msg);// 转换键盘消息
        DispatchMessage(&msg);// 分发消息到窗口过程
        if (msg.message == WM_KEYDOWN) {//键盘按键被按下
            TRACE("msg:%08X wparam:%08X lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
            if (msg.wParam == 0x41) {//按a退出
                break;
            }

        }
    }
    
    ShowCursor(true);
    //ShowWindow() 是Windows API函数，用于控制窗口的显示状态
    //SW_SHOW 参数表示以正常大小显示窗口
    //前面的::表示调用全局命名空间中的函数
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);//显示任务栏
    dlg.DestroyWindow();

    _endthreadex(0);
    return 0;
}
//锁机
int LockMachine()
{
    if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
        //_beginthread(threadLockDlg, 0, NULL);
        _beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &threadid);
        TRACE("\r\nthreadid=%d\r\n", threadid);
    }
    CPacket pack(7, NULL, 0);
    CServerSocket::getInstance()->SendData(pack);
    return 0;
}

//解锁
int UnLockMachine()
{
    //向锁机线程发送退出消息
    PostThreadMessage(threadid, WM_KEYDOWN, 0, 0);
    CPacket pack(7, NULL, 0);
    CServerSocket::getInstance()->SendData(pack);

    return 0;
}   

//删除文件
int DeleteLocalFile()
{
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    TCHAR sPath[MAX_PATH] = _T("");//创建宽字符数组用于存储转换后的文件路径
    // mbstowcs(sPath, strPath.c_str(), strPath.size());//中文容易出现乱码
    MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(), sPath, sizeof(sPath)/sizeof(TCHAR));//将多字节字符串转换为宽字符字符串
    DeleteFile(sPath);
    CPacket pack(9, NULL, 0);
    bool ret = CServerSocket::getInstance()->SendData(pack);
    TRACE("DeleteLocalFile ret = %d\r\n", ret);

    return 0;
}



//连接测试
int TestConnect()
{
    CPacket pack(1981, NULL, 0);
    int ret = CServerSocket::getInstance()->SendData(pack);
    TRACE("Send ret = %d\r\n", ret);

    return 0;
}

//处理命令
int ExecuteCommand(int nCmd)
{
    int ret = 0;
    switch (nCmd) {
    case 1: //先查看磁盘分区，顺带写Dump函数
        ret = MakeDriverInfo();
        break;
    case 2: //查看指定目录下的文件
        ret = MakeDirectoryInfo();
        break;
    case 3: //打开文件
        ret = RunFile();
        break;
    case 4://下载文件
        ret = DownloadFile();
        break;
    case 5: //鼠标操作
        ret = MouseEvent();
        break;
    case 6: //发送屏幕内容,本质就是给控制端发送屏幕的截图
        ret = SendScreen();
        break;
    case 7://锁机
        ret = LockMachine();
        break;
    case 8://解锁
        ret = UnLockMachine();
        break;
    case 9: //删除文件
        ret = DeleteLocalFile();
        break;
    case 1981://连接测试
        ret = TestConnect();
        break;
    }
    return ret;
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
            // TODO: 在此处为应用程序的行为编写代码。
            CServerSocket* pserver = CServerSocket::getInstance();
            int count = 0;
            if (pserver->InitSocket() == false) {
                MessageBox(NULL, _T("网络初始化异常，请检查网络!"), _T("网络初始化失败!"), MB_OK | MB_ICONERROR);
                exit(0);
            }
            while (CServerSocket::getInstance() != NULL) {
                if (pserver->AcceptClient() == false) {
                    if (count >= 3) {
                        MessageBox(NULL, _T("多次无法正常接入用户，结束程序!"), _T("接入用户失败!"), MB_OK | MB_ICONERROR);
	  					exit(0);
                    }
                    MessageBox(NULL, _T("无法正常接入用户，正在重试!"), _T("接入用户失败!"), MB_OK | MB_ICONERROR);
                    count++;
                }
                TRACE("AcceptClient True!\r\n");
       
                int ret = pserver->DealCommand();
                TRACE("DealComand ret = %d\r\n", ret);
                if (ret > 0) {
                    ret = ExecuteCommand(ret);
                    TRACE("main: ExecuteCommand returned %d\r\n", ret);
                    if (ret != 0) {
                        TRACE("执行命令失败：%d ret = %d\r\n", pserver->GetPacket().sCmd, ret);
                    }
                    TRACE("main: Closing client connection\r\n");
                    pserver->CloseClient();
                }
            }

            //文件需求 - 观察、打开、下载、删除

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
