#include "Tray.h"

#include "norwegianwoodstyle.h"
#include "settingsfrm.h"
#include "MissingsFrm.h"
#include "RecordFrm.h"
#include "CheckFrm.h"

#include <QtGui>
/*
#include <QTimer>
#include <QMessageBox>
#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>

#include <QSettings>
#include <QFileInfo>
//#include <QSound>
*/

#include "IsOnline.h"


int weekday(int y, int m, int d)
{
    bool bLeapYear = ((y & 3) == 0 && (y % 100 != 0 || y % 400 == 0));
    const int r[] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};
    return (y + (y >> 2) - y / 100 + y / 400 + d + (bLeapYear && m > 2 ? r[m - 1] + 1 : r[m - 1]) - 1) % 7;
}

bool reccmp(const stRecord &r1, const stRecord &r2)
{
    return r1.time < r2.time;
}


cTray::cTray()
    : QWidget(NULL)
    , m_pCfg(NULL)
    , m_AudioOutput(Phonon::VideoCategory)
{
    setWindowIcon(QIcon(":/res/BIGICON.ico"));
    setWindowTitle("至尊沙漏");

    m_pSettingsFrm = NULL;
    m_pMissingsFrm = NULL;
    m_pRecordFrm = NULL;
    m_pCheckFrm = NULL;

    m_pTimer = NULL;

    m_bRun = false;

    m_nCheckInternet = 0;
    m_nOnlineStat = -1;
    m_pIsOnline = NULL;

    // Sounds

    m_audioOutputPath = Phonon::createPath(&m_MediaObject, &m_AudioOutput);
}

cTray::~cTray()
{
    if (m_pCfg)
        delete m_pCfg;

    if (m_pSettingsFrm)
        delete m_pSettingsFrm;

    if (m_pMissingsFrm)
        delete m_pMissingsFrm;

    if (m_pRecordFrm)
        delete m_pRecordFrm;

    if (m_pCheckFrm)
        delete m_pCheckFrm;

    if (m_pTimer)
        delete m_pTimer;

    for (int i = m_MsgBoxes.size() - 1; i >= 0; --i)
        delete m_MsgBoxes[i];

    if (m_pIsOnline)
        delete m_pIsOnline;

    if (m_nCheckInternet > 0)
        wsacleanup();
}

int cTray::Setup()
{
    LoadIni();

    if (m_pCfg->value("/config/autorun", 1).toInt() == 1)
    {
        setAutoRun();
    }

    m_bPause = false;

    changeStyle(m_pCfg->value("/config/style", "Pagefold").toString());

    m_pacVisit = new QAction("访问主页", this);
    connect(m_pacVisit, SIGNAL(triggered()), this, SLOT(visit()));
    //---
    m_pacAdd = new QAction("添加提醒", this);
    connect(m_pacAdd, SIGNAL(triggered()), this, SLOT(add()));
    m_pacCheck = new QAction("查看提醒", this);
    connect(m_pacCheck, SIGNAL(triggered()), this, SLOT(check()));
    m_pacConfig = new QAction("设置", this);
    connect(m_pacConfig, SIGNAL(triggered()), this, SLOT(config()));
    //---
    m_pacPause = new QAction("暂停运行", this);
    connect(m_pacPause, SIGNAL(triggered()), this, SLOT(pause()));
    m_pacAllSound = new QAction("关闭所有声音", this);
    connect(m_pacAllSound, SIGNAL(triggered()), this, SLOT(switchSound()));
    m_pacAllTurn = new QAction("关闭所有自动切换", this);
    connect(m_pacAllTurn, SIGNAL(triggered()), this, SLOT(switchTurn()));
    //---
    m_pacQuit = new QAction("退出", this);
    connect(m_pacQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

    m_pMenu = new QMenu(this);
    m_pMenu->addAction(m_pacVisit);
    m_pMenu->addSeparator();
    m_pMenu->addAction(m_pacAdd);
    m_pMenu->addAction(m_pacCheck);
    m_pMenu->addAction(m_pacConfig);
    m_pMenu->addSeparator();
    m_pMenu->addAction(m_pacPause);
    m_pMenu->addAction(m_pacAllSound);
    m_pMenu->addAction(m_pacAllTurn);
    m_pMenu->addSeparator();
    m_pMenu->addAction(m_pacQuit);

    m_pIcon = new QSystemTrayIcon(this);
    m_pIcon->setIcon(QIcon(":/res/BIGICON.ico"));
    m_pIcon->setToolTip(getToolTip());
    m_pIcon->setContextMenu(m_pMenu);

    connect(m_pIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    m_pIcon->show();
    ShowMessage(NULL, "至尊沙漏开始运行...", NoIcon);//QSystemTrayIcon::MessageIcon(0)


    LoadDB(qApp->applicationDirPath() + "/db.dat");

    int i = m_Records.size() - 1;
    for (; i >= 0; --i)
    {
        stRecord &rec = m_Records[i];
        if (rec.time < 0x7777777777777777LL)
            break;

        if (rec.once)
        {
            //if (rec.onceType == nextRun)
            //{
                doRemind(i);
                RemoveRecord(i);
            //}
        }
        else// if (rec.ruleType == perRun)
        {
            doRemind(i);
        }
    }
    for (; i >= 0; --i)
    {
        stRecord &rec = m_Records[i];
        if (rec.time < 0x7777777777777775LL)
            break;

        if (m_nCheckInternet <= 0)
        {
            wsastartup();
            m_pIsOnline = new cIsOnline(m_mutexIsOnline, m_bIsOnline);
            m_pIsOnline->start(QThread::IdlePriority);
        }
        ++m_nCheckInternet;
    }

    CheckOverdueRecords();

    //SetMessage(NULL, "距离下一次提醒还有", NoIcon);

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(mainloop()));
    //m_pTimer->start(0);
    m_pTimer->start(20);

    return 0;
}


void cTray::ShowMessage(const QString &title, const QString &msg, TrayMessageIcon icon, int msecs)
{
    SetMessage(title, msg, icon, msecs);
    showMessage();
}


void cTray::SetMessage(const QString &title, const QString &msg, TrayMessageIcon icon, int msecs)
{
    m_Message.title = title;
    m_Message.msg = msg;
    m_Message.icon = (QSystemTrayIcon::MessageIcon)icon;
    m_Message.msecs = msecs;
}


void cTray::AddRecord(stRecord &rec, bool modify)
{
    if (!rec.once)
        calcNextTime(rec);
    if (!modify)
        m_Records.append(rec);
    qSort(m_Records.begin(), m_Records.end(), reccmp);

    updateDbFile();

    if (rec.time == 0x7777777777777776LL || rec.time == 0x7777777777777775LL)
    {
        if (m_nCheckInternet <= 0)
        {
            wsastartup();
            if (!m_pIsOnline)
                m_pIsOnline = new cIsOnline(m_mutexIsOnline, m_bIsOnline);
            m_pIsOnline->start(QThread::IdlePriority);
        }
        ++m_nCheckInternet;
    }

    // Message

    stMsg msg = m_Message;

    char str[128];
    if (rec.time == 0x7777777777777777LL)
    {
        strcpy(str, "将在下次启动本程序时提醒");
    }
    else if (rec.time == 0x7777777777777776LL)
    {
        strcpy(str, "将在下次连入互联网那一刻提醒");
    }
    else if (rec.time == 0x7777777777777775LL)
    {
        strcpy(str, "将在下次断开互联网那一刻提醒");
    }
    else
    {
        QDateTime dt;
        dt.setMSecsSinceEpoch(rec.time);
        sprintf(str, "将在 %04d年%02d月%02d日 %02d:%02d:%02d 时提醒",
                dt.date().year(),
                dt.date().month(),
                dt.date().day(),
                dt.time().hour(),
                dt.time().minute(),
                dt.time().second());
    }
    ShowMessage(modify ? "提醒被修改" : "新提醒", str);
    m_Message = msg;

    UpdateTrayMessage();
}


void cTray::UpdateRecord(stRecord& rec)
{
    AddRecord(rec, true);
}


void cTray::EditRecord(int i, bool fromMissing)
{
    if (!m_pRecordFrm)
        m_pRecordFrm = new RecordFrm(this);

    m_pRecordFrm->Setup(&m_Records[i], fromMissing);

    if (fromMissing)
        RemoveOnce(i);
}


void cTray::SortRecords()
{
    qSort(m_Records.begin(), m_Records.end(), reccmp);
}


void cTray::RemoveRecord(int i)
{
    m_Records.removeAt(i);
    updateDbFile();
}


void cTray::RemoveOnce(int i)
{
    if (m_Records[i].once)
        RemoveRecord(i);
    else
    {
        calcNextTime(m_Records[i]);
        SortRecords();
        updateDbFile();
    }
}

void cTray::AfterProcMissing()
{
    m_bRun = true;
    UpdateTrayMessage();
}


void cTray::UpdateTrayMessage()
{
    if (m_Records.size() < 1)
    {
        SetMessage(NULL, "未设置任何提醒", NoIcon);
        return;
    }

    char str[128];

    if (m_Records[0].time == 0x7777777777777777LL)
    {
        strcpy(str, "下一次提醒在下次启动本程序时");
    }
    else if (m_Records[0].time == 0x7777777777777776LL)
    {
        strcpy(str, "下一次提醒在下次连入互联网那一刻");
    }
    else if (m_Records[0].time == 0x7777777777777775LL)
    {
        strcpy(str, "下一次提醒在下次断开互联网那一刻");
    }
    else
    {
        QDateTime dt;
        dt.setMSecsSinceEpoch(m_Records[0].time);
        sprintf(str, "下一次提醒在 %04d年%02d月%02d日 %02d:%02d:%02d",
                dt.date().year(),
                dt.date().month(),
                dt.date().day(),
                dt.time().hour(),
                dt.time().minute(),
                dt.time().second());
    }
    SetMessage(NULL, str, NoIcon);
}


void cTray::LoadIni()
{
    if (QFileInfo("./config.ini").exists())
    {
        m_pCfg = new QSettings("./config.ini", QSettings::IniFormat);
    }
    else
    {
        m_pCfg = new QSettings("./config.ini", QSettings::IniFormat);
        m_pCfg->setValue("/config/autorun", 1);
        m_pCfg->setValue("/config/style", "Pagefold");
        m_pCfg->setValue("/config/defaultSound", "./sounds/ring.mp3");
        m_pCfg->setValue("/switch/sound", 1);
        m_pCfg->setValue("/switch/turn", 1);
    }

    m_switchSound = (m_pCfg->value("/switch/sound", 1).toInt() == 1);
    m_switchTurn  = (m_pCfg->value("/switch/turn", 1).toInt() == 1);
}


int cTray::LoadDB(const QString &DBFilePath)
{
    FILE *fp = fopen(DBFilePath.toAscii(), "rb");
    if (!fp)
        return -1;
    int n;
    n = 0;
    fread(&n, 1, 1, fp);
    if (n != 'z')
        return -1;
    n = 0;
    fread(&n, 1, 1, fp);
    if (n != 's')
        return -1;
    n = 0;
    fread(&n, 2, 1, fp);// version

    switch (n)
    {
    case (1 << 8) :
    {
        fread(&n, 4, 1, fp);
        if (n <= 0)
            return n;
        m_Records.reserve(m_Records.size() + n);
        stRecord rec;
        stRecord_v1 old;
        for (int i = 0; i < n; ++i)
        {
            fread(&old, sizeof(old), 1, fp);

            rec.time = old.time;
            strcpy(rec.name, old.name);
            rec.createtime = old.createtime;
            rec.lasttime = old.createtime;

            rec.once = old.once;
            if (rec.once)
            {
                rec.onceType = (old.delay ? delay : trueTime);
            }
            else
            {
                rec.ruleType = old.rule;
            }

            rec.cd = old.cd;
            rec.y = old.y;
            rec.m = old.m;
            rec.w = old.w;
            rec.d = old.d;
            rec.h = old.h;
            rec.mi = old.mi;

            rec.popupWindow = old.popupWindow;
            strcpy(rec.text, old.text);
            rec.playSound = old.playSound;
            if (rec.playSound)
                strcpy(rec.soundfile, old.soundfile);
            rec.autoTurn = old.autoTurn;
            rec.runCmd = old.runCmd;
            if (rec.runCmd)
                strcpy(rec.cmd, old.cmd);

            m_Records.append(rec);
        }
        break;
    }

    case ((1 << 8) | 1) :
    {
        fread(&n, 4, 1, fp);
        if (n <= 0)
            return n;
        m_Records.reserve(m_Records.size() + n);
        stRecord rec;
        for (int i = 0; i < n; ++i)
        {
            fread(&rec, sizeof(rec), 1, fp);
            m_Records.append(rec);
        }
        break;
    }

    default:
        fclose(fp);
        return -1;
    }

    fclose(fp);

    qSort(m_Records.begin(), m_Records.end(), reccmp);

    return n;
}


void cTray::CheckOverdueRecords()
{
    m_bRun = false;
    SetMessage(NULL, "请处理掉错过的提醒", NoIcon);
    if (m_Records.size() < 1 || m_Records[0].time >= QDateTime::currentMSecsSinceEpoch())
    {
        AfterProcMissing();
    }
    else
    {
        m_pMissingsFrm = new MissingsFrm(this);
        m_pMissingsFrm->Setup();
    }
}


void cTray::visit()
{
    QDesktopServices::openUrl(QUrl("http://zi-jin.com/archives/462"));
}


void cTray::add()
{
    if (!m_pRecordFrm)
    {
        m_pRecordFrm = new RecordFrm(this);
        m_pRecordFrm->Setup();
    }
    else
        m_pRecordFrm->activateWindow();
}


void cTray::check()
{
    if (!m_pCheckFrm)
    {
        m_pCheckFrm = new CheckFrm(this);
        m_pCheckFrm->Setup();
    }
    else
        m_pRecordFrm->activateWindow();
}


void cTray::config()
{
    if (!m_pSettingsFrm)
    {
        m_pSettingsFrm = new SettingsFrm(this);
        m_pSettingsFrm->Setup();
    }
    else
        m_pRecordFrm->activateWindow();
}


void cTray::pause()
{
    if (m_bPause)
    {
        CheckOverdueRecords();
        m_bPause = false;
        m_pacPause->setText("暂停运行");
        stMsg msg = m_Message;
        if (m_bRun)
        {
            ShowMessage("恢复", "至尊沙漏已恢复运行...");
        }
        else
        {
            ShowMessage("恢复-注意", "已取消暂停，但在处理完错过的提醒前，提醒功能仍不会开启！", Warning);
        }

        m_Message = msg;
    }
    else
    {
        m_bPause = true;
        m_pacPause->setText("恢复运行");
        ShowMessage("暂停", "至尊沙漏已暂停运行...");
    }
}


void cTray::switchSound()
{
    if (m_pCfg->value("/switch/sound", 1).toInt() == 1)
    {
        m_pCfg->setValue("/switch/sound", 0);
        m_switchSound = false;
        m_pacAllSound->setText("开启声音功能");
    }
    else
    {
        m_pCfg->setValue("/switch/sound", 1);
        m_switchSound = true;
        m_pacAllSound->setText("关闭所有声音");
    }
    m_pIcon->setToolTip(getToolTip());
}


void cTray::switchTurn()
{
    if (m_pCfg->value("/switch/turn", 1).toInt() == 1)
    {
        m_pCfg->setValue("/switch/turn", 0);
        m_switchTurn = false;
        m_pacAllTurn->setText("开启自动切换功能");
    }
    else
    {
        m_pCfg->setValue("/switch/turn", 1);
        m_switchTurn = true;
        m_pacAllTurn->setText("关闭所有自动切换");
    }
    m_pIcon->setToolTip(getToolTip());
}


void cTray::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        break;

    case QSystemTrayIcon::DoubleClick:
        if (m_pRecordFrm)
            m_pRecordFrm->activateWindow();
        if (m_pSettingsFrm)
            m_pSettingsFrm->activateWindow();
        if (m_pMissingsFrm)
            m_pMissingsFrm->activateWindow();
    case QSystemTrayIcon::MiddleClick:
        showMessage();
        break;

    default:
        ;
    }
}


void cTray::showMessage()
{
    m_pIcon->showMessage(m_Message.title, m_Message.msg, m_Message.icon, m_Message.msecs);
}


QString cTray::getToolTip()
{
    QString s = "至尊沙漏";
    s.append("\r\n声音：");
    s.append(m_pCfg->value("/switch/sound", 1) == 1 ? "开启" : "关闭");
    s.append("\r\n自动切换：");
    s.append(m_pCfg->value("/switch/turn", 1) == 1 ? "开启" : "关闭");
    return s;
}


void cTray::changeStyle(const QString &style)
{
    if (style == "Pagefold")
    {
        //QFile file(":/qss/" + sheetName.toLower() + ".qss");
        QFile file(":/res/pagefold/pagefold.qss");
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());

        qApp->setStyleSheet(styleSheet);
    }
    else if (style == "Wood")
    {
        qApp->setStyle(new NorwegianWoodStyle);
        qApp->setStyleSheet("");
    }
    /*
    switch (style)
    {
    case "Pagefold":
        qApp->setStyleSheet(":/res/pagefold/pagefold.qss");
        break;

    case "Wood":
        QApplication::setStyle(new NorwegianWoodStyle);
        break;

    default:
        ;
    }*/
}


void cTray::calcNextTime(stRecord &rec)
{
    int y, m, d;
    y = QDate::currentDate().year();
    QDateTime dt;
    if (rec.ruleType == perHour)
        rec.h = QTime::currentTime().hour();
    if (rec.ruleType == perMinute)
    {
        rec.h = QTime::currentTime().hour();
        rec.mi = QTime::currentTime().minute();
    }
    dt.setTime(QTime(rec.h, rec.mi));
    switch (rec.ruleType)
    {
    case perYear:
        {
            do
            {
                m = rec.m;
                if (rec.w == 0)
                    d = rec.d;
                else
                {
                    d = rec.d - (weekday(y, m, 1) + 1);
                    if (d < 0)
                        d += 7;
                    d += 1 + (rec.w - 1) * 7;
                }
                dt.setDate(QDate(y, m, d));
                ++y;
            } while (dt < QDateTime::currentDateTime());

            rec.time = dt.toMSecsSinceEpoch();
            break;
        }

    case perMonth:
        {
            m = QDate::currentDate().month();
            do
            {
                if (rec.w == 0)
                    d = rec.d;
                else
                {
                    d = rec.d - (weekday(y, m, 1) + 1);
                    if (d < 0)
                        d += 7;
                    d += 1 + (rec.w - 1) * 7;
                }
                dt.setDate(QDate(y, m, d));
                ++m;
                if (m > 12)
                {
                    ++y;
                    m = 1;
                }
            } while (dt < QDateTime::currentDateTime());

            rec.time = dt.toMSecsSinceEpoch();
            break;
        }

    case perWeek:
        {
            m = QDate::currentDate().month();
            d = QDate::currentDate().day();
            int i = rec.d - (weekday(y, m, d) + 1);
            if (i < 0)
                i += 7;
            dt.setDate(QDate::currentDate());
            dt = dt.addDays(i);
            if (dt < QDateTime::currentDateTime())
                dt = dt.addDays(7);

            rec.time = dt.toMSecsSinceEpoch();
            break;
        }

    case perDay:
        {
            dt.setDate(QDate::currentDate());
            //m = QDate::currentDate().month();
            //d = QDate::currentDate().day();
            //dt.setDate(QDate(y, m, d));
            if (dt < QDateTime::currentDateTime())
                dt = dt.addDays(1);

            rec.time = dt.toMSecsSinceEpoch();
            break;
        }

    case perHour:
        {
            m = QDate::currentDate().month();
            d = QDate::currentDate().day();
            dt.setDate(QDate(y, m, d));
            if (dt < QDateTime::currentDateTime())
                rec.time = dt.addSecs(60 * 60).toMSecsSinceEpoch();
            else
                rec.time = dt.toMSecsSinceEpoch();
            break;
        }

    case perMinute:
        {
            m = QDate::currentDate().month();
            d = QDate::currentDate().day();
            dt.setDate(QDate(y, m, d));
            if (dt < QDateTime::currentDateTime())
                rec.time = dt.addSecs(60).toMSecsSinceEpoch();
            else
                rec.time = dt.toMSecsSinceEpoch();
            break;
        }

    case interval:
        {
            dword n = QDateTime::currentMSecsSinceEpoch() - rec.createtime;
            dword l = (((rec.y * 365 + rec.m * 28 + rec.w * 7 + rec.d) * 24 + rec.h) * 60 + rec.mi) * 60 * 1000;
            int t = n / l;
            dt.setMSecsSinceEpoch(rec.createtime);
            dt = dt.addYears(t * rec.y);
            dt = dt.addMonths(t * rec.m);
            dt = dt.addDays(t * (rec.d + 7 * rec.w));
            dt = dt.addSecs(t * (60 * 60 * rec.h + 60 * rec.mi));

            do
            {
                dt = dt.addYears(rec.y);
                dt = dt.addMonths(rec.m);
                dt = dt.addDays(rec.d + 7 * rec.w);
                dt = dt.addSecs(60 * 60 * rec.h + 60 * rec.mi);
            } while (dt < QDateTime::currentDateTime());

            rec.time = dt.toMSecsSinceEpoch();
            break;
        }
    }
}


/*
void cTray::createDbFile()
{
    FILE *fp = fopen("db.dat", "wb");
    if (!fp)
        return;
    int n = 'z';
    fwrite(&n, 1, 1, fp);
    n = 's';
    fwrite(&n, 1, 1, fp);
    n = (1 << 8);
    fwrite(&n, 2, 1, fp);
    n = 0;
    fwrite(&n, 4, 1, fp);
    fclose(fp);
}
*/


void cTray::updateDbFile()
{
    FILE *fp = fopen((qApp->applicationDirPath() + "/db.dat").toAscii(), "wb");
    if (!fp)
        return;
    int n = 'z';
    fwrite(&n, 1, 1, fp);
    n = 's';
    fwrite(&n, 1, 1, fp);
    n = ((1 << 8) | 1);
    fwrite(&n, 2, 1, fp);
    n = m_Records.size();
    fwrite(&n, 4, 1, fp);

    for (int i = 0; i < n; ++i)
    {
        fwrite(&m_Records[i], sizeof(stRecord), 1, fp);
    }
    fclose(fp);
}

void cTray::doRemind(int i)
{
    stRecord& rec = m_Records[i];
    stMsg msg = m_Message;
    ShowMessage("提醒", QString("[") + rec.name + "]时间已到！");
    m_Message = msg;
    if (rec.popupWindow)
    {
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Information, "提醒", rec.text, QMessageBox::Ok, this);
        m_MsgBoxes.append(msgBox);
        msgBox->show();
    }
    if (m_switchSound && rec.playSound)
    {
        if (rec.soundfile[0])
        {
            m_MediaObject.setCurrentSource(Phonon::MediaSource(rec.soundfile));
            //QSound::play(rec.soundfile);
        }
        else
        {
            m_MediaObject.setCurrentSource(Phonon::MediaSource(
                    m_pCfg->value("/config/defaultSound", "./sounds/ring.mp3").toString()
                    ));
            //QSound::play(m_pCfg->value("/config/defaultSound", "./sounds/ring.mp3").toString());
        }
        m_MediaObject.play();
    }
    if (m_switchTurn && rec.autoTurn)
    {
         ; //
    }
    if (rec.runCmd)
    {
        system(rec.cmd);
    }
}


void cTray::mainloop()
{
    bool onIdle = (!m_bRun || m_bPause || m_Records.size() < 1);

    qint64 curTime = QDateTime::currentMSecsSinceEpoch();

    if (m_nCheckInternet > 0)
    {
        m_mutexIsOnline.lock();
        m_pIsOnline->check(!onIdle);

        if (!onIdle)
        {
            if (m_bIsOnline)
            {
                if (m_nOnlineStat != 1)
                {
                    m_nOnlineStat = 1;
                    for (int i = m_Records.size() - 1; i >= 0; --i)
                    {
                        stRecord &rec = m_Records[i];
                        if (rec.time > 0x7777777777777776LL)
                            continue;
                        if (rec.time < 0x7777777777777776LL)
                            break;

                        doRemind(i);
                        if (rec.once)
                        {
                            RemoveRecord(i);
                            --m_nCheckInternet;
                            if (m_nCheckInternet <= 0)
                            {
                                m_pIsOnline->exit();
                                wsacleanup();
                            }
                        }
                    }
                }
            }
            else if (m_nOnlineStat != 0)
            {
                m_nOnlineStat = 0;
                for (int i = m_Records.size() - 1; i >= 0; --i)
                {
                    stRecord &rec = m_Records[i];
                    if (rec.time > 0x7777777777777775LL)
                        continue;
                    if (rec.time < 0x7777777777777775LL)
                        break;

                    doRemind(i);
                    if (rec.once)
                    {
                        RemoveRecord(i);
                        --m_nCheckInternet;
                        if (m_nCheckInternet <= 0)
                        {
                            m_pIsOnline->exit();
                            wsacleanup();
                        }
                    }
                }
            }
        }

        m_mutexIsOnline.unlock();
    }

    if (!onIdle)
    {
        if (m_Records[0].time <= curTime)
        {
            doRemind(0);
            RemoveOnce(0);
            UpdateTrayMessage();
        }
        else if (m_Records[0].time > curTime + 10000)
        {
            onIdle = true;
        }
    }

    if (onIdle)
    {
        for (int i = m_MsgBoxes.size() - 1; i >= 0; --i)
        {
            if (m_MsgBoxes[i]->isHidden())
            {
                delete m_MsgBoxes[i];
                m_MsgBoxes.removeAt(i);
            }
        }
    }
}


void cTray::setAutoRun(bool bAutoRun)
{
    QSettings *reg = new QSettings(
            "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
            , QSettings::NativeFormat);
    if (bAutoRun)
    {
        QString path = qApp->applicationFilePath();
        path.replace('/', '\\');
        reg->setValue("ZhiZunShaLou", path);
    }
    else
        reg->remove("ZhiZunShaLou");
    delete reg;
}
