
#include <QtCore>

bool wsastartup();
void wsacleanup();

class cIsOnline : public QThread
{
public:
    cIsOnline(QMutex& IsOnlineMutex, bool& bIsOnline, QObject *parent = NULL)
        : QThread(parent)
        , m_mutex(IsOnlineMutex)
        , m_bIsOnline(bIsOnline)
        , m_bCheck(true)
    {
    }

    void run();

    inline void check(bool bCheck)
    {
        m_bCheck = bCheck;
    }
    inline bool check()
    {
        return m_bCheck;
    }

protected:
    QMutex& m_mutex;
    bool&   m_bIsOnline;
    bool    m_bCheck;
};
