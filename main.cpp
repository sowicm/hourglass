#include <QApplication>
#include <QtCore/QTextCodec>
#include <QDir>

#include "Tray.h"

#ifdef QS_WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
    QDir::setCurrent(qApp->applicationDirPath());

#if defined(QS_WIN32) && 1
    HANDLE hMutex = CreateMutexA(NULL, TRUE, "ZhiZunShaLou");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(hMutex);
        MessageBoxA(NULL, "干嘛同时运行2个呢？", "Oh~Oh!", MB_OK | MB_ICONINFORMATION);
        return 0;
    }
#endif

    Q_INIT_RESOURCE(resource);

    QApplication a(argc, argv);
#if defined(QS_WIN32)
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    //QTextCodec::codecForName("UTF-8"));
#endif

    a.setApplicationName("Sowicm's Hourglass");
    a.setOrganizationName("sowicm");
    a.setOrganizationDomain("sowicm.com");

    a.setQuitOnLastWindowClosed(false);

    cTray tray;
    tray.Setup();

    int ret = a.exec();
#if  defined(QS_WIN32) && 1
    CloseHandle(hMutex);
#endif

    return ret;
}
