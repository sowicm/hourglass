
#include "IsOnline.h"


#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <C:\Program Files\Microsoft SDKs\Windows\v7.0A\include\SensAPI.h>
#pragma comment(lib, "SensAPI.lib")

#if 0
#include <wininet.h>
#pragma comment(lib, "Wininet.lib")
#endif

bool wsastartup()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        return false;
    }
    return true;
}
void wsacleanup()
{
    WSACleanup();
}

bool IsOnline()
{
    DWORD dwFlags;
    BOOL m_bAlive = IsNetworkAlive(&dwFlags);
    if (!m_bAlive)
        return false;

    hostent *phe = gethostbyname("www.sina.com");
    return (phe != NULL);

#if 0
    SOCKET sck = socket(PF_INET, SOCK_STREAM, 0);
    if (sck == INVALID_SOCKET)
        return false;

    in_addr ip_addr;
    *(DWORD*)&ip_addr = *(DWORD*)phe->h_addr_list[0];

    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr = ip_addr;

    if (connect(sck, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        return false;

    char request[] =
        "GET /   HTTP/1.1\r\n"
        "Host:www.baidu.com\r\n"
        "Accept:*/*\r\n"
        "User-Agent:Mozilla/4.0   (compatible;   MSIE   11.00;   Windows 6.1)\r\n"
        "Pragma:   no-cache\r\n"
        "Cache-Control:   no-cache\r\n"
        "Connection: Close\r\n\r\n";

    if (send(sck, request, strlen(request), 0) == SOCKET_ERROR)
        return false;

    char buf[32];
    bool ret = (recv(sck, buf, 16, 0) > 0);
    closesocket(sck);
    return ret;
#endif

#if 0
    return InternetCheckConnectionA("www.sina.com", 0, 0);
#endif
}


void cIsOnline::run()
{
    while (true)
    {
        if (m_bCheck)
        {
            m_mutex.lock();
            m_bIsOnline = IsOnline();
            m_mutex.unlock();
        }
        Sleep(200);
    }
}
