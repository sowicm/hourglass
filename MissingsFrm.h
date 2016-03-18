#ifndef MISSINGSFRM_H
#define MISSINGSFRM_H

#include <QWidget>
#include <QDateTime>
#include <QModelIndex>

class cTray;

class QSortFilterProxyModel;
class QStandardItemModel;

namespace Ui {
    class MissingsFrm;
}

class MissingsFrm : public QWidget
{
    Q_OBJECT

public:
    explicit MissingsFrm(cTray *pMain = 0);
    ~MissingsFrm();

public:
    void Setup();

private:
    void closeEvent(QCloseEvent *);

private:
    void addRecord(const QString& name, bool once, const QVariant& time, const QVariant& createTime,
                   const QString& cmdLine);

private:
    cTray*          m_pMain;
    QSortFilterProxyModel *m_pProxyModel;
    QStandardItemModel    *m_pModel;

private:
    Ui::MissingsFrm *ui;

private slots:
    void on_treeView_doubleClicked(QModelIndex index);
};

#endif // MISSINGSFRM_H
