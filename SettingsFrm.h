#ifndef SETTINGSFRM_H
#define SETTINGSFRM_H

#include <QWidget>

class cTray;

class QCompleter;

namespace Ui {
    class SettingsFrm;
}

class SettingsFrm : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsFrm(cTray *pMain);
    ~SettingsFrm();

private:
    cTray           *m_pMain;
    QCompleter      *m_pCompleter;

private:
    Ui::SettingsFrm *ui;

public:
    void Setup();

private:
    void closeEvent(QCloseEvent *);

private slots:
    void on_fileButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void on_styleCombo_currentIndexChanged(QString );
};

#endif // SETTINGSFRM_H
