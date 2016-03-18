#include <QtGui/QApplication>
#include <QtCore/QTextCodec>
#include <QDir>

#include "Tray.h"

#include <windows.h>

int main(int argc, char *argv[])
{
    QDir::setCurrent(qApp->applicationDirPath());

#if 1
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
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    //QTextCodec::codecForName("UTF-8"));

    a.setApplicationName("至尊沙漏");
    a.setOrganizationName("至尊明帅");
    a.setOrganizationDomain("zi-jin.com");

    a.setQuitOnLastWindowClosed(false);

    cTray tray;
    tray.Setup();

    int ret = a.exec();
#if 1
    CloseHandle(hMutex);
#endif

    return ret;
}
