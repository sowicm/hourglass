#include "RecordFrm.h"
#include "ui_RecordFrm.h"

#include "Tray.h"

#include <QtGui>
#include <QCompleter>
#include <QFileSystemModel>
#include <QFileDialog>
#include <QMessageBox>

RecordFrm::RecordFrm(cTray *pMain) :
    QWidget(NULL)//, Qt::FramelessWindowHint)
    , m_pMain(pMain)
    , ui(new Ui::RecordFrm)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(size());

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

RecordFrm::~RecordFrm()
{
    delete m_pCompleter;
    delete ui;
}


void RecordFrm::Setup(stRecord *rec, bool bClone)
{
    m_pRecord = rec;
    if (bClone)
        m_pRecord = NULL;

    // ----- Init UI Begin -----
    ui->tabWidget->setCurrentIndex(0);

    //ui->dateTimeEdit->setCalendarPopup(true);
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

    ui->rcdCombo->setCurrentIndex(0);
    on_rcdCombo_currentIndexChanged(0);
    ui->rcdCombo->setDisabled(true);    // only cd for now

    ui->rmCombo->setCurrentIndex(0);;
    on_rmCombo_currentIndexChanged(0);

    ui->rddRadio->setChecked(true);
    ui->rvhSpin->setRange(0, 23);
    ui->rvmiSpin->setRange(0, 59);

    ui->rySpin->setRange(0, 99);
    ui->rmSpin->setRange(0, 99);
    ui->rwSpin->setRange(0, 99);
    ui->rdSpin->setRange(0, 99);
    ui->rhSpin->setRange(0, 99);
    ui->rmiSpin->setRange(0, 99);

    ui->popupCheck->setChecked(true);
    ui->soundCheck->setChecked(true);
    ui->defaultSoundRadio->setChecked(true);
    ui->autoturnCheck->setChecked(true);
    ui->runcmdCheck->setChecked(false);
    ui->cmdEdit->setDisabled(true);
    ui->trayCheck->setChecked(true);
    ui->trayCheck->setDisabled(true);
    // ----- Init UI End -----

    if (rec)
    {
        if (rec->once)
        {
            ui->tabWidget->setCurrentIndex(0);
            switch (rec->onceType)
            {
            case trueTime:
            {
                ui->ovRadio->setChecked(true);
                QDateTime dt;
                dt.setMSecsSinceEpoch(rec->time);
                ui->dateTimeEdit->setDateTime(dt);
                break;
            }
            case delay:
            {
                ui->odRadio->setChecked(true);
                qint64 r = rec->time - QDateTime::currentMSecsSinceEpoch();
                if (r <= 0)
                    ui->timeEdit->setTime(QTime(0, 0));
                else
                {
                    int h, mi, s, ms;
                    h = r / (60 * 60 * 1000);
                    r -= h * (60 * 60 * 1000);
                    mi = r / (60 * 1000);
                    r -= mi * (60 * 1000);
                    s = r / 1000;
                    ms = r - s * 1000;
                    ui->timeEdit->setTime(QTime(h, mi, s, ms));
                }
                break;
            }
            case nextRun:
            {
                ui->osRadio->setChecked(true);
                break;
            }
            case nextOnline:
            {
                ui->ooRadio->setChecked(true);
                break;
            }
            case nextOffline:
            {
                ui->ofRadio->setChecked(true);
                break;
            }
            }
        }
        else
        {
            ui->tabWidget->setCurrentIndex(1);

            switch (rec->ruleType)
            {
            case perYear:
                ui->ryRadio->setChecked(true);
                break;

            case perMonth:
                ui->rmRadio->setChecked(true);
                break;

            case perWeek:
                ui->rwRadio->setChecked(true);
                break;

            case perDay:
                ui->rdRadio->setChecked(true);
                break;

            case perHour:
                ui->rhRadio->setChecked(true);
                break;

            case perMinute:
                ui->rmiRadio->setChecked(true);
                break;

            case interval:
                ui->riRadio->setChecked(true);
                break;

            case perRun:
                ui->rsRadio->setChecked(true);
                break;

            case perOnline:
                ui->roRadio->setChecked(true);
                break;

            case perOffline:
                ui->rfRadio->setChecked(true);
                break;
            }

            switch (rec->ruleType)
            {
            case perYear:
                ui->rmCombo->setCurrentIndex(rec->m - 1);

            case perMonth:
                if (rec->w == 0)
                    ui->rddRadio->setChecked(true);
                else
                    ui->rwCombo->setCurrentIndex(rec->w - 1);

            case perWeek:
                if (rec->w == 0)
                    ui->rdCombo->setCurrentIndex(rec->d - 1);
                else
                    ui->rwdCombo->setCurrentIndex(rec->d - 1);

            case perDay:
                ui->rvhSpin->setValue(rec->h);

            case perHour:
                ui->rvmiSpin->setValue(rec->mi);
                break;

            case interval:
                ui->rySpin->setValue(rec->y);
                ui->rmSpin->setValue(rec->m);
                ui->rwSpin->setValue(rec->w);
                ui->rdSpin->setValue(rec->d);
                ui->rhSpin->setValue(rec->h);
                ui->rmiSpin->setValue(rec->mi);
                break;
            }
        }


        if (rec->popupWindow)
        {
            ui->popupCheck->setChecked(true);
            ui->textEdit->setPlainText(rec->text);
        }
        else
        {
            ui->popupCheck->setChecked(false);
        }

        if (rec->playSound)
        {
            ui->soundCheck->setChecked(true);
            if (rec->soundfile[0] == '\0')
            {
                ui->defaultSoundRadio->setChecked(true);
            }
            else
            {
                ui->specSoundRadio->setChecked(true);
                ui->soundEdit->setText(rec->soundfile);
            }
        }
        else
        {
            ui->soundCheck->setChecked(false);
        }

        ui->autoturnCheck->setChecked(rec->autoTurn);

        if (rec->runCmd)
        {
            ui->runcmdCheck->setChecked(true);
            ui->cmdEdit->setEnabled(true);
            ui->cmdEdit->setText(rec->cmd);
        }
        else
        {
            ui->runcmdCheck->setChecked(false);
        }

        ui->nameEdit->setText(rec->name);
    }

    show();
    //raise();
    //activateWindow();
}


void RecordFrm::closeEvent(QCloseEvent *e)
{
    QWidget::closeEvent(e);

    m_pMain->m_pRecordFrm = NULL;
    delete this;
}


void RecordFrm::on_rcdCombo_currentIndexChanged(int index)
{
    QStringList list;
    const char cn[][16] = {"一", "二", "三", "四", "五", "六", "七", "八", "九", "十", "冬", "腊"};
    if (index == 0)
    {
        for (int i = 1; i <= 12; ++i)
        {
            list << QString::number(i) + "月";
        }
    }
    else if (index == 1)
    {
        for (int i = 0; i < 12; ++i)
        {
            list << QString(cn[i]) + "月";
        }
    }

    ui->rmCombo->clear();
    ui->rmCombo->addItems(list);

    // TODO： 自动转换农历公历日期
}

void RecordFrm::on_rmCombo_currentIndexChanged(int index)
{
    const int days[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int cd = ui->rcdCombo->currentIndex();
    QStringList list;
    if (cd == 0)
    {
        for (int i = 1; i <= days[index]; ++i)
            list << QString::number(i) + "日";
    }
    ui->rdCombo->clear();
    ui->rdCombo->addItems(list);
}

void RecordFrm::on_ryRadio_clicked()
{
    ui->rmCombo->setDisabled(false);
    ui->rdGroupBox->setDisabled(false);
    ui->rvhSpin->setDisabled(false);
    ui->rvmiSpin->setDisabled(false);

    ui->rddRadio->setDisabled(false);
    ui->rdCombo->setDisabled(false);
    ui->rdwRadio->setDisabled(false);
    ui->rwCombo->setDisabled(false);
    ui->rwdCombo->setDisabled(false);

    //ui->rsRadio->setChecked(false);
}

void RecordFrm::on_rmRadio_clicked()
{
    ui->rmCombo->setDisabled(true);
    ui->rdGroupBox->setDisabled(false);
    ui->rvhSpin->setDisabled(false);
    ui->rvmiSpin->setDisabled(false);


    ui->rddRadio->setDisabled(false);
    ui->rdCombo->setDisabled(false);
    ui->rdwRadio->setDisabled(false);
    ui->rwCombo->setDisabled(false);
    ui->rwdCombo->setDisabled(false);

    //ui->rsRadio->setChecked(false);
}

void RecordFrm::on_rwRadio_clicked()
{
    ui->rmCombo->setDisabled(true);
    ui->rdGroupBox->setDisabled(false);
    ui->rvhSpin->setDisabled(false);
    ui->rvmiSpin->setDisabled(false);

    ui->rdwRadio->setChecked(true);

    ui->rddRadio->setDisabled(true);
    ui->rdCombo->setDisabled(true);
    ui->rdwRadio->setDisabled(true);
    ui->rwCombo->setDisabled(true);
    ui->rwdCombo->setDisabled(false);

    //ui->rsRadio->setChecked(false);
}

void RecordFrm::on_rdRadio_clicked()
{
    ui->rmCombo->setDisabled(true);
    ui->rdGroupBox->setDisabled(true);
    ui->rvhSpin->setDisabled(false);
    ui->rvmiSpin->setDisabled(false);

    //ui->rsRadio->setChecked(false);
}

void RecordFrm::on_rhRadio_clicked()
{
    ui->rmCombo->setDisabled(true);
    ui->rdGroupBox->setDisabled(true);
    ui->rvhSpin->setDisabled(true);
    ui->rvmiSpin->setDisabled(false);

    //ui->rsRadio->setChecked(false);
}

void RecordFrm::on_rmiRadio_clicked()
{
    ui->rmCombo->setDisabled(true);
    ui->rdGroupBox->setDisabled(true);
    ui->rvhSpin->setDisabled(true);
    ui->rvmiSpin->setDisabled(true);

    //ui->rsRadio->setChecked(false);
}
//----------------------------------------------------------

void RecordFrm::on_popupCheck_clicked(bool checked)
{
    ui->textEdit->setDisabled(!checked);
}

void RecordFrm::on_soundCheck_clicked(bool checked)
{
    ui->soundGroupBox->setDisabled(!checked);
}

void RecordFrm::on_runcmdCheck_clicked(bool checked)
{
    ui->cmdEdit->setDisabled(!checked);
}

void RecordFrm::on_soundButton_clicked()
{
    ui->specSoundRadio->setChecked(true);
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        ui->soundEdit->setText(fileName);
    }
}


void RecordFrm::on_okButton_clicked()
{
    if (strlen(ui->textEdit->document()->toPlainText().toLocal8Bit().data()) > 255)
    {
        QMessageBox::critical(this, "Sorry", "窗口提示文字太长！请确保长度不超过255个字节");
        return;
    }
    if (strlen(ui->soundEdit->text().toLocal8Bit().data()) > 260)
    {
        QMessageBox::critical(this, "Are you kidding me?", "声音文件有这么长的路径吗？");
        return;
    }
    if (strlen(ui->cmdEdit->text().toLocal8Bit().data()) > 127)
    {
        QMessageBox::critical(this, "Sorry", "Cmd过长，请确保不要超过127个字节");
        return;
    }
    if (strlen(ui->nameEdit->text().toLocal8Bit().data()) > 63)
    {
        QMessageBox::critical(this, "Sorry", "备注名太长！请确保不要超过63个字节");
        return;
    }


    stRecord record;
    stRecord *rec;

    if (m_pRecord)
    {
        rec = m_pRecord;
        rec->lasttime = QDateTime::currentMSecsSinceEpoch();
    }
    else
    {
        rec = &record;
        rec->lasttime = rec->createtime = QDateTime::currentMSecsSinceEpoch();
    }

    if (ui->tabWidget->currentIndex() == 0)
    {
        rec->once = true;
        if (ui->ovRadio->isChecked())
        {
            rec->onceType = trueTime;
            rec->time = ui->dateTimeEdit->dateTime().toMSecsSinceEpoch();
        }
        else if (ui->odRadio->isChecked())
        {
            rec->onceType = delay;
            rec->time = QDateTime::currentDateTime().addSecs(
                    ((ui->timeEdit->time().hour() * 60) +
                    ui->timeEdit->time().minute()) * 60 +
                    ui->timeEdit->time().second()
                    ).toMSecsSinceEpoch();
        }
        else if (ui->osRadio->isChecked())
        {
            rec->onceType = nextRun;
            rec->time = 0x7777777777777777LL; // 7ff...LL
        }
        else if (ui->ooRadio->isChecked())
        {
            rec->onceType = nextOnline;
            rec->time = 0x7777777777777776LL;
        }
        else if (ui->ofRadio->isChecked())
        {
            rec->onceType = nextOffline;
            rec->time = 0x7777777777777775LL;
        }
        else
        {
            QMessageBox::information(this, "Oh~Oh!", "请指定[定时提醒]、[延时提醒]等具体提醒时间", QMessageBox::Ok);
            return;
        }
    }
    else
    {
        rec->once = false;
        if (ui->ryRadio->isChecked())
        {
            rec->ruleType = perYear;
        }
        else if (ui->rmRadio->isChecked())
        {
            rec->ruleType = perMonth;
        }
        else if (ui->rwRadio->isChecked())
        {
            rec->ruleType = perWeek;
        }
        else if (ui->rdRadio->isChecked())
        {
            rec->ruleType = perDay;
        }
        else if (ui->rhRadio->isChecked())
        {
            rec->ruleType = perHour;
        }
        else if (ui->rmiRadio->isChecked())
        {
            rec->ruleType = perMinute;
        }
        else if (ui->riRadio->isChecked())
        {
            rec->ruleType = interval;
        }
        else if (ui->rsRadio->isChecked())
        {
            rec->ruleType = perRun;
            rec->time = 0x7777777777777777LL;
        }
        else if (ui->roRadio->isChecked())
        {
            rec->ruleType = perOnline;
            rec->time = 0x7777777777777776LL;
        }
        else if (ui->rfRadio->isChecked())
        {
            rec->ruleType = perOffline;
            rec->time = 0x7777777777777775LL;
        }
        else
        {
            QMessageBox::information(this, "Oh~Oh!", "请指定提醒规律！", QMessageBox::Ok);
            return;
        }

        rec->cd = 1;

        switch (rec->ruleType)
        {
        case perYear:
            rec->m = ui->rmCombo->currentIndex() + 1;

        case perMonth:
            rec->w = (ui->rddRadio->isChecked() ? 0 : ui->rwCombo->currentIndex() + 1);

        case perWeek:
            rec->d = (ui->rddRadio->isChecked() ? ui->rdCombo->currentIndex() + 1 : ui->rwdCombo->currentIndex() + 1);

        case perDay:
            rec->h = ui->rvhSpin->value();

        case perHour:
            rec->mi = ui->rvmiSpin->value();
            break;

        case interval:
            rec->y = ui->rySpin->value();
            rec->m = ui->rmSpin->value();
            rec->w = ui->rwSpin->value();
            rec->d = ui->rdSpin->value();
            rec->h = ui->rhSpin->value();
            rec->mi = ui->rmiSpin->value();
            break;
        }
    }

    if (ui->popupCheck->isChecked())
    {
        rec->popupWindow = 1;
        strcpy(rec->text, ui->textEdit->document()->
               toPlainText().toLocal8Bit().data());
    }
    else
    {
        rec->popupWindow = 0;
    }

    if (ui->soundCheck->isChecked())
    {
        rec->playSound = 1;
        if (ui->defaultSoundRadio->isChecked())
        {
            rec->soundfile[0] = '\0';
        }
        else
        {
            strcpy(rec->soundfile, ui->soundEdit->text().toLocal8Bit().data());
        }
    }
    else
    {
        rec->playSound = 0;
    }

    rec->autoTurn = ui->autoturnCheck->isChecked();

    if (ui->runcmdCheck->isChecked())
    {
        rec->runCmd = 1;
        strcpy(rec->cmd, ui->cmdEdit->text().toLocal8Bit().data());
    }
    else
    {
        rec->runCmd = 0;
    }


    strcpy(rec->name, ui->nameEdit->text().toLocal8Bit().data());

    if (!m_pRecord)
        m_pMain->AddRecord(record);
    else
    {
        m_pMain->UpdateRecord(*m_pRecord);
    }

    close();
    //m_pMain->m_pRecordFrm = NULL;
    //delete this;
}

void RecordFrm::on_cancelButton_clicked()
{
    close();
    //m_pMain->m_pRecordFrm = NULL;
    //delete this;
}

void RecordFrm::on_dateTimeEdit_dateTimeChanged(QDateTime)
{
    ui->ovRadio->setChecked(true);
}

void RecordFrm::on_timeEdit_timeChanged(QTime date)
{
    ui->odRadio->setChecked(true);
}
