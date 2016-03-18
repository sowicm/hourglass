#ifndef CHECKFRM_H
#define CHECKFRM_H

#include <QWidget>
#include <QDateTime>
#include <QModelIndex>

class cTray;

class QSortFilterProxyModel;
class QStandardItemModel;

namespace Ui {
    class CheckFrm;
}

class CheckFrm : public QWidget
{
    Q_OBJECT

public:
    explicit CheckFrm(cTray *pMain = 0);
    ~CheckFrm();

public:
    void Setup();

private:
    void closeEvent(QCloseEvent *);

private:
    void addRecord(const QString& name, bool once, const QVariant& time, const QVariant& createTime,
                   const QString& cmdLine);

private:
    cTray *m_pMain;
    QSortFilterProxyModel *m_pProxyModel;
    QStandardItemModel    *m_pModel;

private:
    Ui::CheckFrm *ui;

private slots:
    void on_lineEdit_textChanged(QString );
    void on_deleteButton_clicked();
    void on_treeView_doubleClicked(QModelIndex index);
    void on_editButton_clicked();
    void on_closeButton_clicked();
    void on_pushButton_clicked();
};

#endif // CHECKFRM_H
