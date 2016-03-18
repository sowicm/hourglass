#include "CheckFrm.h"
#include "ui_checkfrm.h"

#include "Tray.h"

#include <QtGui>
/*
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QMessageBox>
*/

CheckFrm::CheckFrm(cTray *pMain) :
        QWidget(NULL)//, Qt::FramelessWindowHint)
        , m_pMain(pMain)
        , ui(new Ui::CheckFrm)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(size());

    m_pProxyModel = new QSortFilterProxyModel;
    m_pProxyModel->setDynamicSortFilter(true);
    m_pProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_pProxyModel->setFilterKeyColumn(0);

    ui->treeView->setRootIsDecorated(false);
    ui->treeView->setAlternatingRowColors(true);
    ui->treeView->setModel(m_pProxyModel);
    ui->treeView->setSortingEnabled(true);

    ui->treeView->sortByColumn(2, Qt::AscendingOrder);

    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_pModel = NULL;
}

CheckFrm::~CheckFrm()
{
    delete ui;

    if (m_pModel)
        delete m_pModel;

    delete m_pProxyModel;
}


void CheckFrm::Setup()
{
    if (!m_pModel)
    {
        m_pModel = new QStandardItemModel(0, 5, this);
        m_pModel->setHeaderData(0, Qt::Horizontal, "��ע");
        m_pModel->setHeaderData(1, Qt::Horizontal, "����");
        m_pModel->setHeaderData(2, Qt::Horizontal, "����ʱ��");
        m_pModel->setHeaderData(3, Qt::Horizontal, "����ʱ��");
        m_pModel->setHeaderData(4, Qt::Horizontal, "��������");
    }

    stRecord *rec;
    QDateTime dt;
    m_pModel->removeRows(0, m_pModel->rowCount());
    QVariant remindtime;
    for (int i = m_pMain->m_Records.size() - 1; i >= 0; --i)
    {
        rec = &m_pMain->m_Records[i];
        if (rec->once)
        {
            switch (rec->onceType)
            {
            case nextRun:
                remindtime = "�´�����ʱ";
                break;

            case nextOnline:
                remindtime = "�´�����Internetʱ";
                break;

            case nextOffline:
                remindtime = "�´δ�Internet�Ͽ�ʱ";
                break;

            default:
                dt.setMSecsSinceEpoch(rec->time);
                remindtime = dt;
                break;
            }
        }
        else
        {
            switch (rec->ruleType)
            {
            case perRun:
                remindtime = "ÿ������ʱ";
                break;

            case perOnline:
                remindtime = "ÿ������Internetʱ";
                break;

            case perOffline:
                remindtime = "ÿ�δ�Internet�Ͽ�ʱ";
                break;

            default:
                dt.setMSecsSinceEpoch(rec->time);
                remindtime = dt;
                break;
            }

        }

        //dt1.setMSecsSinceEpoch(rec->time);
        dt.setMSecsSinceEpoch(rec->createtime);
        addRecord(rec->name,
                  rec->once,
                  remindtime,
                  dt,
                  rec->runCmd ? rec->cmd : "");
    }

    m_pProxyModel->setSourceModel(m_pModel);
    ui->treeView->setColumnWidth(0, 180);
    ui->treeView->setColumnWidth(1, 60);
    ui->treeView->setColumnWidth(2, 125);
    ui->treeView->setColumnWidth(3, 125);

    ui->label->setText("����" + QString::number(m_pMain->m_Records.size()) + "����¼");


    show();
    //raise();
    //activateWindow();
}


void CheckFrm::closeEvent(QCloseEvent *e)
{
    QWidget::closeEvent(e);

    m_pMain->m_pCheckFrm = NULL;
    delete this;
}


void CheckFrm::addRecord(const QString &name, bool once, const QVariant &time, const QVariant &createTime,
                         const QString &cmdLine)
{
    m_pModel->insertRow(0);
    m_pModel->setData(m_pModel->index(0, 0), name);
    m_pModel->setData(m_pModel->index(0, 1), once ? "��������" : "��������");
    m_pModel->setData(m_pModel->index(0, 2), time);
    m_pModel->setData(m_pModel->index(0, 3), createTime);
    m_pModel->setData(m_pModel->index(0, 4), cmdLine);
}

void CheckFrm::on_closeButton_clicked()
{
    close();
    //m_pMain->m_pCheckFrm = NULL;
    //delete this;
}

void CheckFrm::on_editButton_clicked()
{
    on_treeView_doubleClicked( /*m_pProxyModel->mapFromSource(*/ui->treeView->currentIndex() );
}

void CheckFrm::on_treeView_doubleClicked(QModelIndex index)
{
    QModelIndex _index = m_pProxyModel->index(index.row(), 3);
    for (int i = m_pMain->m_Records.size() - 1; i >=0 ; --i)
    {
        if (m_pProxyModel->data(_index).toDateTime().toMSecsSinceEpoch() == m_pMain->m_Records[i].createtime)
        {
            m_pMain->EditRecord(i);
            break;
        }
    }
}

void CheckFrm::on_deleteButton_clicked()
{
    if (QMessageBox::warning(
            this,
            "��ȷ����",
            "ȷ��Ҫɾ����������𣿲������ɻָ���",
            QMessageBox::Yes | QMessageBox::No
            ) == QMessageBox::Yes)
    {
        QModelIndex index = /*m_pProxyModel->mapFromSource(*/ui->treeView->currentIndex();
        QModelIndex _index = m_pProxyModel->index(index.row(), 3);
        for (int i = m_pMain->m_Records.size() - 1; i >=0 ; --i)
        {
            if (m_pProxyModel->data(_index).toDateTime().toMSecsSinceEpoch() == m_pMain->m_Records[i].createtime)
            {
                m_pMain->RemoveRecord(i);
                m_pProxyModel->removeRow(_index.row());
                ui->label->setText("����" + QString::number(m_pMain->m_Records.size()) + "����¼");
                break;
            }
        }
    }
}

void CheckFrm::on_lineEdit_textChanged(QString filterPattern)
{
    //QRegExp regExp(filterPattern, Qt::CaseInsensitive, QRegExp::FixedString);
    m_pProxyModel->setFilterFixedString(filterPattern);
}

void CheckFrm::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, QString(), qApp->applicationDirPath(), "�����ļ�(*.dat)");
    if (!fileName.isEmpty())
    {
        int n = m_pMain->LoadDB(fileName);
        if (n < 0)
            QMessageBox::critical(this, "Oh~Oh��", "���ļ����ǡ�����ɳ©�������ļ����ļ����𻵣�");
        else
            QMessageBox::information(this, "����ɹ�", "�ѳɹ�����" + QString::number(n) + "���¼");
    }
}
