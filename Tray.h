#ifndef TRAY_H
#define TRAY_H

#include "defines.h"
#include <QWidget>
#include <QSystemTrayIcon>
#include <QMutex>

#include <phonon/audiooutput.h>
#include <phonon/MediaObject>

class QSettings;
class QAction;
class QMenu;
class QSystemTrayIcon;
class QMessageBox;

class cIsOnline;

class SettingsFrm;
class MissingsFrm;
class RecordFrm;
class CheckFrm;


enum TrayMessageIcon { NoIcon, Information, Warning, Critical };


enum OnceType{ trueTime, delay, nextRun, nextOnline, nextOffline };
//enum RuleType_v1{ perYear_v1, perMonth_v1, perWeek_v1, perDay_v1, perHour_v1, perMinute_v1, interval_v1 };
enum RuleType{ perYear, perMonth, perWeek, perDay, perHour, perMinute, interval, perRun, perOnline, perOffline };



struct stRecord //_v1_1
{
    qint64 time;
    char name[64];
    qint64 createtime;
    qint64 lasttime;

    bool once;
    union {
        OnceType onceType;
        RuleType ruleType;
    };
    //union {
    //    dword time; // for once
    //    struct { // for non-once
            uint cd : 1;
            uint y : 16;
            uint m : 16;
            uint w : 16;
            uint d : 16;
            uint h : 16;
            uint mi : 16;
    //    };
    //};

    bool popupWindow;
    char text[256];
    bool playSound;
    char soundfile[261];
    bool autoTurn;
    bool runCmd;
    char cmd[128];
};

struct stRecord_v1
{
    qint64 time;
    char name[64];
    qint64 createtime;

    bool once;
    union {
        bool delay; // for once
        RuleType rule; // for non-once
    };
    //union {
    //    dword time; // for once
    //    struct { // for non-once
            uint cd : 1;
            uint y : 16;
            uint m : 16;
            uint w : 16;
            uint d : 16;
            uint h : 16;
            uint mi : 16;
    //    };
    //};

    bool popupWindow;
    char text[256];
    bool playSound;
    char soundfile[261];
    bool autoTurn;
    bool runCmd;
    char cmd[128];
};


class cTray : public QWidget
{
friend class SettingsFrm;
friend class CheckFrm;
    Q_OBJECT

public:
                                    cTray();
                                    ~cTray();

public:
    int                             Setup();
    void                            ShowMessage(const QString& title,
                                                const QString& msg,
                                                TrayMessageIcon icon = Information,
                                                int msecs = 10000);
    void                            SetMessage(const QString& title,
                                               const QString& msg,
                                               TrayMessageIcon icon = Information,
                                               int msecs = 10000);
    void                            AddRecord(stRecord& rec, bool modify = false);
    void                            EditRecord(int i, bool fromMissing = false);
    void                            UpdateRecord(stRecord& rec);
    void                            SortRecords();
    void                            RemoveRecord(int i);
    void                            RemoveOnce(int i);
    void                            AfterProcMissing();
    void                            UpdateTrayMessage();

private:
    void                            LoadIni();
    int                             LoadDB(const QString& DBFilePath);
    void                            CheckOverdueRecords();

private slots:
    void                            visit();
    void                            add();
    void                            check();
    void                            config();
    void                            pause();
    void                            switchSound();
    void                            switchTurn();

    void                            mainloop();

    void                            iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void                            showMessage();
    QString                         getToolTip();
    void                            changeStyle(const QString& style);
    void                            calcNextTime(stRecord& rec);
    //void                            createDbFile();
    void                            updateDbFile();
    void                            doRemind(int i);
    void                            setAutoRun(bool bAutoRun = true);

public:
    QTimer                         *m_pTimer;
    bool                            m_bRun;
    bool                            m_bPause;
    bool                            m_switchSound;
    bool                            m_switchTurn;
    QSettings                      *m_pCfg;
    QAction                        *m_pacVisit,
                                   *m_pacPause,
                                   *m_pacAdd,
                                   *m_pacCheck,
                                   *m_pacConfig,
                                   *m_pacAllSound,
                                   *m_pacAllTurn,
                                   *m_pacQuit;
    QMenu                          *m_pMenu;
    QSystemTrayIcon                *m_pIcon;
    struct stMsg {
        QString title;
        QString msg;
        QSystemTrayIcon::MessageIcon icon;
        int msecs;
    } m_Message;

    QList<stRecord>                 m_Records;// QList<stRecord*> faster?
    QList<QMessageBox*>             m_MsgBoxes;

    Phonon::MediaObject             m_MediaObject;
    Phonon::AudioOutput             m_AudioOutput;
    Phonon::Path                    m_audioOutputPath;

    int                             m_nCheckInternet;
    bool                            m_bIsOnline;
    int                             m_nOnlineStat;
    QMutex                          m_mutexIsOnline;
    cIsOnline                      *m_pIsOnline;

    SettingsFrm                    *m_pSettingsFrm;
    MissingsFrm                    *m_pMissingsFrm;
    RecordFrm                      *m_pRecordFrm;
    CheckFrm                       *m_pCheckFrm;
};

#endif // TRAY_H
