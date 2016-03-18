#ifndef RECORDFRM_H
#define RECORDFRM_H

#include <QWidget>
#include <QDateTime>

class cTray;
struct stRecord;

class QCompleter;

namespace Ui {
    class RecordFrm;
}

class RecordFrm : public QWidget
{
    Q_OBJECT

public:
    explicit RecordFrm(cTray *pMain = 0);
    ~RecordFrm();

private:
    cTray               *m_pMain;
    stRecord*            m_pRecord; // use for edit record
    QCompleter          *m_pCompleter;

public:
    void Setup(stRecord* rec = NULL, bool bClone = false);

private:
    void closeEvent(QCloseEvent *);

private:
    Ui::RecordFrm *ui;

private slots:
    void on_timeEdit_timeChanged(QTime date);
    void on_dateTimeEdit_dateTimeChanged(QDateTime date);
    void on_cancelButton_clicked();
    void on_okButton_clicked();
    void on_soundButton_clicked();
    void on_runcmdCheck_clicked(bool checked);
    void on_soundCheck_clicked(bool checked);
    void on_popupCheck_clicked(bool checked);
    void on_rmiRadio_clicked();
    void on_rhRadio_clicked();
    void on_rdRadio_clicked();
    void on_rwRadio_clicked();
    void on_rmRadio_clicked();
    void on_ryRadio_clicked();
    void on_rmCombo_currentIndexChanged(int index);
    void on_rcdCombo_currentIndexChanged(int index);
};

#endif // RECORDFRM_H
