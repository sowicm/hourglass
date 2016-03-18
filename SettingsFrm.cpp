#include "SettingsFrm.h"
#include "ui_settingsfrm.h"

#include "Tray.h"

#include <QtGui>
/*
#include <QSettings>
#include <QFileDialog>
#include <QCompleter>
#include <QFileSystemModel>
#include <QMessageBox>
*/


SettingsFrm::SettingsFrm(cTray *pMain) :
        QWidget(NULL)//, Qt::FramelessWindowHint)
        , m_pMain(pMain)
        , ui(new Ui::SettingsFrm)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(size());

    ui->styleCombo->addItem("Pagefold");
    ui->styleCombo->addItem("Wood");

    //UpdateData();

    m_pCompleter = new QCompleter(this);
    m_pCompleter->setMaxVisibleItems(9);
    QFileSystemModel *fsModel = new QFileSystemModel(m_pCompleter);
    fsModel->setRootPath("");
    m_pCompleter->setModel(fsModel);
    m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_pCompleter->setWrapAround(true);

    ui->soundEdit->setCompleter(m_pCompleter);
}

SettingsFrm::~SettingsFrm()
{
    delete m_pCompleter;
    delete ui;
}


void SettingsFrm::Setup()
{
    // autorun
    ui->autorunCheck->setCheckState(m_pMain->m_pCfg->value("/config/autorun", 1).toInt() ? Qt::Checked : Qt::Unchecked);

    // defaultsound
    ui->soundEdit->setText(m_pMain->m_pCfg->value("/config/defaultSound", "./sounds/ring.mp3").toString());

    // style combo
    int i = ui->styleCombo->findText(m_pMain->m_pCfg->value("/config/style", "Pagefold").toString());
    ui->styleCombo->setCurrentIndex(i < 0 ? 0 : i);

    show();
    //raise();
    //activateWindow();
}


void SettingsFrm::closeEvent(QCloseEvent *e)
{
    QWidget::closeEvent(e);

    m_pMain->m_pSettingsFrm = NULL;
    delete this;
}


void SettingsFrm::on_styleCombo_currentIndexChanged(QString str)
{
    m_pMain->changeStyle(str);
}

void SettingsFrm::on_pushButton_clicked()
{
    if (strlen(ui->soundEdit->text().toAscii().data()) > 260)
    {
        QMessageBox::critical(this, "Are you kidding me?", "声音文件有这么长的路径吗？");
        return;
    }

    m_pMain->m_pCfg->setValue("/config/autorun", ui->autorunCheck->isChecked() ? 1 : 0);
    m_pMain->m_pCfg->setValue("config/style", ui->styleCombo->currentText());
    m_pMain->m_pCfg->setValue("/config/defaultSound", ui->soundEdit->text());
    m_pMain->setAutoRun(ui->autorunCheck->isChecked());
    close();

    //m_pMain->m_pSettingsFrm = NULL;
    //delete this;
}

void SettingsFrm::on_pushButton_2_clicked()
{
    m_pMain->changeStyle(m_pMain->m_pCfg->value("/config/style", "Pagefold").toString());
    close();//hide();

    //m_pMain->m_pSettingsFrm = NULL;
    //delete this;
}

void SettingsFrm::on_fileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        ui->soundEdit->setText(fileName);
    }
}
