#include "MissingsFrm.h"
#include "ui_missingsfrm.h"

#include "Tray.h"

#include <QtGui>
/*
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
*/

MissingsFrm::MissingsFrm(cTray *pMain) :
        QWidget(NULL, Qt::FramelessWindowHint)
        , m_pMain(pMain)
        , ui(new Ui::MissingsFrm)
{
    ui->setupUi(this);

    setFixedSize(size());

    m_pProxyModel = new QSortFilterProxyModel;
    m_pProxyModel->setDynamicSortFilter(true);

    ui->treeView->setRootIsDecorated(false);
    ui->treeView->setAlternatingRowColors(true);
    ui->treeView->setModel(m_pProxyModel);
    ui->treeView->setSortingEnabled(true);

    ui->treeView->sortByColumn(2, Qt::AscendingOrder);

    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_pModel = NULL;
}

MissingsFrm::~MissingsFrm()
{
    delete ui;

    if (m_pModel)
        delete m_pModel;

    delete m_pProxyModel;
}


void MissingsFrm::Setup()
{
    if (!m_pModel)
    {
        m_pModel = new QStandardItemModel(0, 4, this);
        m_pModel->setHeaderData(0, Qt::Horizontal, "备注");
        m_pModel->setHeaderData(1, Qt::Horizontal, "类型");
        m_pModel->setHeaderData(2, Qt::Horizontal, "提醒时间");
        m_pModel->setHeaderData(3, Qt::Horizontal, "创建时间");
    }

    stRecord *rec;
    QDateTime dt1, dt2;
    m_pModel->removeRows(0, m_pModel->rowCount());
    qint64 curTime = QDateTime::currentMSecsSinceEpoch();
    for (int i = 0; i < m_pMain->m_Records.size(); ++i)
    {
        rec = &m_pMain->m_Records[i];
        if (rec->time > curTime)
            break;
        dt1.setMSecsSinceEpoch(rec->time);
        dt2.setMSecsSinceEpoch(rec->createtime);
        addRecord(rec->name,
                  rec->once,
                  dt1,
                  dt2,
                  rec->runCmd ? rec->cmd : "");
    }

    m_pProxyModel->setSourceModel(m_pModel);
    ui->treeView->setColumnWidth(0, 180);
    ui->treeView->setColumnWidth(1, 60);
    ui->treeView->setColumnWidth(2, 125);
    ui->treeView->setColumnWidth(3, 125);


    show();
    //raise();
    //activateWindow();
}


void MissingsFrm::closeEvent(QCloseEvent *e)
{
    QWidget::closeEvent(e);

    m_pMain->m_pMissingsFrm = NULL;
    delete this;
}


void MissingsFrm::addRecord(const QString &name, bool once, const QVariant& time, const QVariant& createTime,
                            const QString &cmdLine)
{
    m_pModel->insertRow(0);
    m_pModel->setData(m_pModel->index(0, 0), name);
    m_pModel->setData(m_pModel->index(0, 1), once ? "当次提醒" : "规律提醒");
    m_pModel->setData(m_pModel->index(0, 2), time);
    m_pModel->setData(m_pModel->index(0, 3), createTime);
    m_pModel->setData(m_pModel->index(0, 4), cmdLine);
}

void MissingsFrm::on_treeView_doubleClicked(QModelIndex index)
{
    QModelIndex _index = m_pProxyModel->index(index.row(), 3);
    for (int i = m_pMain->m_Records.size() - 1; i >=0 ; --i)
    {
        if (m_pProxyModel->data(_index).toDateTime().toMSecsSinceEpoch() == m_pMain->m_Records[i].createtime)
        {
            m_pMain->EditRecord(i, true);
            break;
        }
    }

    m_pProxyModel->removeRow(index.row());
    //m_pModel->removeRow(index.row());


    if (m_pProxyModel->rowCount() < 1)//m_pModel->rowCount() < 1)
    {
        m_pMain->AfterProcMissing();

        close();
        //m_pMain->m_pMissingsFrm = NULL;
        //delete this;
    }
}
