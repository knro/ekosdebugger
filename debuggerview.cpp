/*******************************************************************************
  Copyright(c) 2020 Ikarus Technologies. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*******************************************************************************/

#include <QDebug>
#include <QTimer>
#include <QStandardItemModel>
#include <QXmlSimpleReader>
#include <QStandardPaths>

#include "driverinfo.h"
#include "userdb.h"
#include "debuggerview.h"
#include "./ui_debuggerview.h"

#define ERRMSG_SIZE    1024

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
DebuggerView::DebuggerView(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DebuggerView)
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QString currentDate = QDate::currentDate().toString("yyyy-MM-dd");
    if(QDir(dataPath + "/kstars/logs/" + currentDate).exists())
        watcher.addPath( dataPath + "/kstars/logs/" + currentDate);
    else
        watcher.addPath( dataPath + "/kstars/logs");

    ui->setupUi(this);
    ui->statusbar->showMessage("Welcome to Ekos Debugger!");
    m_MountModel = new QStandardItemModel(this);

    QSettings settings;
    bool restartINDI = settings.value("indi/restart", true).toBool();
    ui->restartINDICB->setChecked(restartINDI);
    connect(ui->restartINDICB, &QCheckBox::toggled, [](bool checked)
    {
        QSettings settings;
        settings.setValue("indi/restart", checked);
    });

    bool restartKStars = settings.value("kstars/restart", true).toBool();
    ui->restartKStarsCB->setChecked(restartKStars);
    connect(ui->restartKStarsCB, &QCheckBox::toggled, [](bool checked)
    {
        QSettings settings;
        settings.setValue("kstars/restart", checked);
    });

    QString kstarsExe = settings.value("kstars/exe", "/usr/bin/kstars").toString();
    ui->KStarsExeField->setText(kstarsExe);

    QString indiExe = settings.value("indi/exe", "/usr/bin/indiserver").toString();
    ui->INDIExeField->setText(indiExe);

    connect(ui->startKStarsB, &QPushButton::clicked, this, &DebuggerView::startKStars);
    connect(ui->stopKStarsB, &QPushButton::clicked, this, &DebuggerView::stopKStars);
    connect(ui->copyKStarsDebugLogB, &QPushButton::clicked, this, &DebuggerView::copyKStarsDebugLog);
    connect(ui->startINDIB, &QPushButton::clicked, this, &DebuggerView::startINDI);
    connect(ui->stopINDIB, &QPushButton::clicked, this, &DebuggerView::stopINDI);
    connect(ui->copyINDIDebugLogB, &QPushButton::clicked, this, &DebuggerView::copyINDIDebugLog);
    connect(ui->copyINDIAppLogB, &QPushButton::clicked, this, &DebuggerView::copyINDIAppLog);
    connect(ui->profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DebuggerView::loadDriverCombo);
    connect(ui->driverCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DebuggerView::createINDIArgs);
    connect(ui->saveKStarsLogsB, &QPushButton::clicked, this, &DebuggerView::saveKStarsLogs);
    connect(ui->saveINDILogsB, &QPushButton::clicked, this, &DebuggerView::saveINDILogs);
    connect(&watcher, &QFileSystemWatcher::directoryChanged, this, &DebuggerView::findLogFile);
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, &DebuggerView::findLogFile);
    connect(ui->clearKStarsDebugLogB, &QPushButton::clicked, this, &DebuggerView::clearKStarsDebugLog);
    connect(ui->clearINDIDebugLogB, &QPushButton::clicked, this, &DebuggerView::clearINDIDebugLog);
    connect(ui->clearINDIAppLogB, &QPushButton::clicked, this, &DebuggerView::clearINDIAppLog);
    connect(ui->restoreDefaultKStarsExeB, &QPushButton::clicked, this, &DebuggerView::restoreDefaultKStarsExe);
    connect(ui->restoreDefaultINDIExeB, &QPushButton::clicked, this, &DebuggerView::restoreDefaultINDIExe);

    readXMLDrivers();
    loadProfiles();
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
DebuggerView::~DebuggerView()
{
    if(m_KStarsProcess != nullptr)
        stopKStars();
    if(m_INDIProcess != nullptr)
        stopINDI();
    delete ui;
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::restoreDefaultINDIExe()
{
    QSettings settings;
    ui->INDIExeField->setText("/usr/bin/indiserver");
    settings.setValue("indi/exe", ui->INDIExeField->text());
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::restoreDefaultKStarsExe()
{
    QSettings settings;
    ui->KStarsExeField->setText("/usr/bin/kstars");
    settings.setValue("kstars/exe", ui->KStarsExeField->text());
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::clearKStarsDebugLog()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear KStars Debug Log", "Are you sure you want to clear the debug log?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        ui->KStarsDebugLog->clear();
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::clearINDIDebugLog()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear INDI Debug Log", "Are you sure you want to clear the debug log?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        ui->INDIDebugLog->clear();
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::clearINDIAppLog()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear INDI App Log", "Are you sure you want to clear the app log?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        ui->INDIAppLog->clear();
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::startKStars()
{
    m_KStarsProcess = new QProcess();
    QStringList args;
    QSettings settings;
    settings.setValue("kstars/exe", ui->KStarsExeField->text());

    if (!QFile::exists(ui->KStarsExeField->text()))
    {
        QMessageBox messageBox;
        messageBox.critical(0, "Error", "KStars was not found in the path specified!");
        return;
    }

    args << "-batch"
         << "-ex" << "handle SIG32 nostop noprint"
         << "-ex" << "handle SIG33 nostop noprint"
         << "-ex" << "run"
         << "-ex" << "bt"
         << ui->KStarsExeField->text();

    ui->startKStarsB->setEnabled(false);
    ui->stopKStarsB->setEnabled(true);
    for (auto &oneButton : ui->modulesButtonGroup->buttons())
        oneButton->setEnabled(false);
    ui->statusbar->showMessage("Started KStars.");

    m_EkosInterfaceCounter = 0;

    // Set which Ekos logging is supposed to be on/off so that the log file is concise and useful for developmer
    connect(m_KStarsProcess, &QProcess::started, this, &DebuggerView::connectEkosDBus);
    //reads output and error process logs and connects them to their corresponding proccesing functions.
    connect(m_KStarsProcess, &QProcess::readyReadStandardOutput, this, &DebuggerView::processKStarsOutput);
    //    connect(m_KStarsProcess, &QProcess::readyReadStandardError, this, &DebuggerView::processKStarsError);
    //catches if KStars process has exited or crashed.
    connect(m_KStarsProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [ = ](int exitCode, QProcess::ExitStatus exitStatus)
    {
        Q_UNUSED(exitCode);
        Q_UNUSED(exitStatus);
        if (ui->stopKStarsB->isEnabled())
        {
            ui->startKStarsB->setEnabled(true);
            ui->stopKStarsB->setEnabled(false);
            for (auto &oneButton : ui->modulesButtonGroup->buttons())
                oneButton->setEnabled(true);
            // Alternative way that sucks and is not accurate
            // since many received signal can be shown in the log due to the log
            // not being cleared after every start of KStars
            //            QString temp = ui->KStarsDebugLog->toPlainText();
            //            bool crashed = false;
            //            bool crashed = temp.contains("\"kstars\" received signal");

            // Sets the cursor to the end of KStarsDebugLog QTextBrowser and selects only
            // the last line, so it extracts [Inferior 1 (process 6753) exited normally]
            // and then checks if it contains "exited normally]"
            // if it contains it, then the program didn't crash, else, it crashed.
            ui->KStarsDebugLog->moveCursor( QTextCursor::End );
            ui->KStarsDebugLog->moveCursor( QTextCursor::StartOfLine, QTextCursor::KeepAnchor );
            QString text = ui->KStarsDebugLog->textCursor().selectedText();
            // deselection
            QTextCursor cursor = ui->KStarsDebugLog->textCursor();
            cursor.movePosition( QTextCursor::End );
            ui->KStarsDebugLog->setTextCursor( cursor );

            bool normally = text.contains("exited normally]");


            if(!normally)
            {
                ui->statusbar->showMessage("KStars crashed.");
                if(ui->restartKStarsCB->isChecked())
                    startKStars();
            }
            else
                ui->statusbar->showMessage("KStars exited normally.");

        }
    });

    m_KStarsProcess->start("gdb", args);
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::stopKStars()
{
    m_KStarsProcess->terminate();
    ui->startKStarsB->setEnabled(true);
    ui->stopKStarsB->setEnabled(false);
    for (auto &oneButton : ui->modulesButtonGroup->buttons())
        oneButton->setEnabled(true);
    ui->statusbar->showMessage("Stopped KStars.");
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::processKStarsOutput()
{
    QString buffer = m_KStarsProcess->readAllStandardOutput();
    buffer = buffer.trimmed();
    ui->KStarsDebugLog->append(buffer);
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::copyKStarsDebugLog()
{
    ui->KStarsDebugLog->selectAll();
    ui->KStarsDebugLog->copy();
    ui->statusbar->showMessage("Copied KStars Debug log.");
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::startINDI()
{
    INDItimestamp = QDateTime::currentDateTime().toString("yy-MM-ddThh-mm-ss");
    m_INDIProcess = new QProcess();
    QSettings settings;
    settings.setValue("indi/exe", ui->INDIExeField->text());

    if (!QFile::exists(ui->INDIExeField->text()))
    {
        QMessageBox messageBox;
        messageBox.critical(0, "Error", "INDI server was not found in the path specified!");
        return;
    }

    QStringList args;
    args << "-batch"
         << "-ex" << "handle SIG32 nostop"
         << "-ex" << "set follow-fork-mode child"
         << "-ex" << "run"
         << "-ex" << "bt"
         << "--args" << ui->INDIExeField->text() << "-r" << "0" << "-v" << INDIArgs;
    ui->startINDIB->setDisabled(true);
    ui->stopINDIB->setDisabled(false);
    ui->profileCombo->setDisabled(true);
    ui->driverCombo->setDisabled(true);
    ui->statusbar->showMessage("Started INDI.");

    connect(m_INDIProcess, &QProcess::readyReadStandardOutput, this, &DebuggerView::processINDIOutput);
    connect(m_INDIProcess, &QProcess::readyReadStandardError, this, &DebuggerView::processINDIError);
    connect(m_INDIProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [ = ](int exitCode, QProcess::ExitStatus exitStatus)
    {
        Q_UNUSED(exitCode);
        Q_UNUSED(exitStatus);
        if (ui->stopINDIB->isEnabled())
        {
            ui->startINDIB->setDisabled(false);
            ui->stopINDIB->setDisabled(true);
            ui->profileCombo->setDisabled(false);
            ui->driverCombo->setDisabled(false);

            // killing zombie processes
            QPointer<QProcess> m_quitINDIProcess = new QProcess();
            m_quitINDIProcess->start("pkill", QStringList() << "indiserver");
            m_quitINDIProcess->waitForFinished();
            m_quitINDIProcess->close();

            // Sets the cursor to the end of INDIDebugLog QTextBrowser and selects only
            // the last line, so it extracts #12 0x000055555555bcee in _start ()
            // and then checks if it contains "#" at index 0
            // if it contains it, then the program crashed, else, it exited normally.
            ui->INDIDebugLog->moveCursor( QTextCursor::End );
            ui->INDIDebugLog->moveCursor( QTextCursor::StartOfLine, QTextCursor::KeepAnchor );
            QString text = ui->INDIDebugLog->textCursor().selectedText();
            // deselection
            QTextCursor cursor = ui->INDIDebugLog->textCursor();
            cursor.movePosition( QTextCursor::End );
            ui->INDIDebugLog->setTextCursor( cursor );

            bool crashed = text[0] == "#" || text.contains("Backtrace");

            if(crashed)
            {
                ui->statusbar->showMessage("INDI crashed.");
                if(ui->restartINDICB->isChecked())
                    startINDI();
            }
            else
                ui->statusbar->showMessage("INDI exited normally.");
        }
    });

    // killing zombie processes
    QPointer<QProcess> m_quitINDIProcess = new QProcess();
    m_quitINDIProcess->start("pkill", QStringList() << "indiserver");
    m_quitINDIProcess->waitForFinished();
    m_quitINDIProcess->close();

    m_INDIProcess->start("gdb", args);
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::stopINDI()
{
    m_INDIProcess->terminate();

    // killing zombie processes
    QPointer<QProcess> m_quitINDIProcess = new QProcess();
    QStringList args;
    args << "indiserver";
    m_quitINDIProcess->start("pkill", args);
    m_quitINDIProcess->waitForFinished();
    m_quitINDIProcess->close();

    ui->startINDIB->setDisabled(false);
    ui->stopINDIB->setDisabled(true);
    ui->profileCombo->setDisabled(false);
    ui->driverCombo->setDisabled(false);
    ui->statusbar->showMessage("Stopped INDI.");
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::processINDIOutput()
{
    QString buffer = m_INDIProcess->readAllStandardOutput();
    buffer = buffer.trimmed();
    ui->INDIDebugLog->append(buffer);
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::processINDIError()
{
    QString buffer = m_INDIProcess->readAllStandardError();
    buffer = buffer.trimmed();
    ui->INDIAppLog->append(buffer);
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::copyINDIDebugLog()
{
    ui->INDIDebugLog->selectAll();
    ui->INDIDebugLog->copy();
    ui->statusbar->showMessage("Copied INDI Debug log.");
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::copyINDIAppLog()
{
    ui->INDIAppLog->selectAll();
    ui->INDIAppLog->copy();
    ui->statusbar->showMessage("Copied INDI Application log.");
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::loadProfiles()
{
    profiles.clear();
    ProfileDB::GetAllProfiles(profiles);

    for (auto &pi : profiles)
    {
        ui->profileCombo->addItem(pi->name);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::loadDriverCombo()
{
    currProfile = nullptr;

    ui->driverCombo->clear();

    // Get current profile
    for (auto &pi : profiles)
    {
        if (ui->profileCombo->currentText() == pi->name)
        {
            currProfile = pi.get();
            break;
        }
    }
    setPi(currProfile);
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::createINDIArgs()
{
    if (ui->driverCombo->currentText() == nullptr)
        return;

    INDIArgs.clear();

    for (auto &grp : driverslist->m_groups)
    {
        for(auto &dev : grp->m_devices)
        {
            if (ui->driverCombo->currentText() == dev[0])
            {
                INDIArgs.append(dev[1]);
                break;
            }
        }
    }
    for(auto &driver : currProfile->drivers)
    {
        if (ui->driverCombo->currentText() == driver)
            continue;
        for (auto &grp : driverslist->m_groups)
        {
            for(auto &dev : grp->m_devices)
            {
                if (driver == dev[0])
                {
                    INDIArgs.append(dev[1]);
                    break;
                }
            }
        }

    }
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::setPi(ProfileInfo * value)
{
    pi = value;

    QMapIterator<QString, QString> i(pi->drivers);

    while (i.hasNext())
    {
        i.next();

        QString key   = i.key();
        QString value = i.value();
        ui->driverCombo->addItem(value);

    }

}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
bool DebuggerView::readXMLDrivers()
{
    // TODO fix it for MacOS
    QDir indiDir("/usr/share/indi");
    QString driverName;

    indiDir.setNameFilters(QStringList() << "indi_*.xml" << "drivers.xml");
    indiDir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList list = indiDir.entryInfoList();

    for (auto &fileInfo : list)
    {
        if (fileInfo.fileName().endsWith(QLatin1String("_sk.xml")))
            continue;
        readXMLDriverList(fileInfo.absoluteFilePath());
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::readXMLDriverList(const QString &driversFile)
{
    QFile file(driversFile);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        qCritical() << "Cannot read file" << file.errorString();
        return;
    }


    XmlDriversListReader xmlReader(driverslist);
    //    xmlReader.read(&file);

    if (!xmlReader.read(&file))
        qWarning() << "Parse error in file " << xmlReader.errorString();
    else
        driverslist->print();
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::saveKStarsLogs()
{
    //    QFile f(KStarsLogFilePath);
    //    QFileInfo fileInfo(f.fileName());
    //    QString KStarsLogFileName(fileInfo.fileName());
    QString timestamp;
    QString KStarsLogFileName = "";
    QString KStarsLogFolderName = "";

    if (KStarsLogFilePath != "")
    {
        QStringList pieces = KStarsLogFilePath.split( "/" );
        KStarsLogFolderName = pieces.value( pieces.length() - 2 );
        KStarsLogFileName = pieces.value( pieces.length() - 1 );
        QRegExp separator("[(_|.)]");
        QStringList parts = KStarsLogFileName.split(separator);
        timestamp = KStarsLogFolderName + "T" + parts[1];
    }
    else
        timestamp = QDateTime::currentDateTime().toString("yy-MM-ddThh-mm-ss");

    QSettings settings;
    QString homePath = settings.value("kstars/savePath", QDir::homePath()).toString();


    QString folderpath;
    folderpath = QFileDialog::getExistingDirectory(this, "Save KStars logs", homePath, QFileDialog::ShowDirsOnly);

    QDir().mkdir(folderpath + "/kstars_logs_" + timestamp);
    QString filepath;
    filepath = folderpath + "/kstars_logs_" + timestamp;
    //qDebug() << filepath;
    if(!filepath.isEmpty() && !filepath.isNull())
    {
        settings.setValue("kstars/savePath", folderpath);
        QString debugLog = ui->KStarsDebugLog->toPlainText();
        //        QString appLog = ui->KStarsAppLog->toPlainText();

        QString debuglogtxt = filepath + "/kstars_debug_log_" + timestamp + ".txt";
        QFile debugfile( debuglogtxt );
        if ( debugfile.open(QIODevice::ReadWrite) )
        {
            QTextStream stream( &debugfile );
            stream << debugLog << endl;
        }
        debugfile.close();

        //        QString applogtxt = filepath + "/kstars_app_log_" + timestamp + ".txt";
        //        QFile appfile( applogtxt );
        //        if ( appfile.open(QIODevice::ReadWrite) )
        //        {
        //            QTextStream stream( &appfile );
        //            stream << appLog << endl;
        //        }
        //        appfile.close();

        //    QElapsedTimer tmr;
        //    tmr.start();
        //        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        //        QString lastlines = "";
        //        QFile file(KStarsLogFilePath);
        //        if(file.open(QIODevice::ReadOnly))
        //        {
        //            file.seek(file.size() - 1);
        //            int count = 0;
        //            int lines = 2000;
        //            while ( (count <= lines) && (file.pos() > 0) )
        //            {
        //                QString ch = file.read(1);
        //                file.seek(file.pos() - 2);
        //                if (ch == "\n")
        //                    count++;
        //            }
        //            lastlines = file.readAll();

        //            //    qDebug() <<"reading took" << tmr.elapsed()<< " ms";
        //            file.close();
        //        }
        if (QFile::exists(filepath + "/" + KStarsLogFileName))
        {
            QFile::remove(filepath + "/" + KStarsLogFileName);
        }

        QFile::copy(KStarsLogFilePath, filepath + "/" + KStarsLogFileName);

        //        QString logtxt = filepath + "/kstars_app_log_" + timestamp + ".txt";
        //        QFile logfile( logtxt );
        //        if ( logfile.open(QIODevice::ReadWrite) )
        //        {
        //            QTextStream stream( &logfile );
        //            stream << lastlines << endl;
        //        }
        //        logfile.close();
        QStringList zipArgs;
        zipArgs << "-r" << "-j" << folderpath + "/kstars_logs_" + timestamp + ".zip" << filepath;
        //        zip -r folderpath/kstars_logs_timestamp.zip filepath
        m_zippingProcess = new QProcess();
        m_zippingProcess->start("zip", zipArgs);
        m_zippingProcess->waitForFinished();
        QString output(m_zippingProcess->readAllStandardOutput());
        qDebug() << output;
        m_zippingProcess->close();

        ui->statusbar->showMessage("Saved KStars logs.");
    }

    //    settings.setValue("kstars/savePath", filepath);
    //    settings.setValue("indi/savePath", 68);


}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::saveINDILogs()
{
    QSettings settings;
    QString homePath = settings.value("indi/savePath", QDir::homePath()).toString();
    QString timestamp;
    if (INDItimestamp == "")
        timestamp = QDateTime::currentDateTime().toString("yy-MM-ddThh-mm-ss");
    else
        timestamp = INDItimestamp;

    QString folderpath;
    folderpath = QFileDialog::getExistingDirectory(this, "Save INDI logs", homePath, QFileDialog::ShowDirsOnly);

    QDir().mkdir(folderpath + "/indi_logs_" + timestamp);
    QString filepath;
    filepath = folderpath + "/indi_logs_" + timestamp;
    qDebug() << filepath;
    if(!filepath.isEmpty() && !filepath.isNull())
    {
        settings.setValue("indi/savePath", folderpath);

        QString debugLog = ui->INDIDebugLog->toPlainText();
        QString appLog = ui->INDIAppLog->toPlainText();

        QString debuglogtxt = filepath + "/indi_debug_log_" + timestamp + ".txt";
        QFile debugfile( debuglogtxt );
        if ( debugfile.open(QIODevice::ReadWrite) )
        {
            QTextStream stream( &debugfile );
            stream << debugLog << endl;
        }
        debugfile.close();

        QString applogtxt = filepath + "/indi_app_log_" + timestamp + ".txt";
        QFile appfile( applogtxt );
        if ( appfile.open(QIODevice::ReadWrite) )
        {
            QTextStream stream( &appfile );
            stream << appLog << endl;
        }
        appfile.close();

        QStringList zipArgs;
        zipArgs << "-r" << "-j" << folderpath + "/indi_logs_" + timestamp + ".zip" << filepath;
        //        zip -r folderpath/indi_logs_timestamp.zip filepath
        m_zippingProcess = new QProcess();
        m_zippingProcess->start("zip", zipArgs);
        m_zippingProcess->waitForFinished();
        QString output(m_zippingProcess->readAllStandardOutput());
        qDebug() << output;
        m_zippingProcess->close();

        ui->statusbar->showMessage("Saved INDI logs.");
    }

    //    settings.setValue("indi/savePath", filepath);


}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::findLogFile(const QString &str)
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QDir logsDir(dataPath + "/kstars/logs");
    QStringList logFolders = logsDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Time);
    QDir logDir(dataPath + "/kstars/logs/" + logFolders[0]);
    QStringList logList = logDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Time);

    if(str == dataPath + "/kstars/logs")
    {
        watcher.addPath(dataPath + "/kstars/logs/" + logFolders[0]);
        KStarsLogFilePath = dataPath + "/kstars/logs/" + logFolders[0] + "/" + logList[0];
    }
    else if(str == dataPath + "/kstars/logs/" + logFolders[0])
    {
        KStarsLogFilePath = dataPath + "/kstars/logs/" + logFolders[0] + "/" + logList[0];
    }

    //qDebug() << watcher.directories();
    //qDebug() << KStarsLogFilePath;
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::connectEkosDBus()
{
    m_EkosInterface = new QDBusInterface("org.kde.kstars",
                                         "/KStars/Ekos",
                                         "org.kde.kstars.Ekos",
                                         QDBusConnection::sessionBus(),
                                         this);

    QVariant const ekosStatus = m_EkosInterface->property("ekosStatus");

    if (!ekosStatus.isValid())
    {
        if (m_EkosInterfaceCounter++ > 12)
        {
            qWarning() << "Failed to hook to Ekos DBus interface. Cannot remote control logging.";
            return;
        }

        // Try in 5 secs
        QTimer::singleShot(5000, this, &DebuggerView::connectEkosDBus);
        return;
    }
    else
        setEkosLogsEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////
void DebuggerView::setEkosLogsEnabled(bool enabled)
{
    QStringList loggers;
    loggers << "FILE" << "VERBOSE" << "INDI" << "FITS" << "CAPTURE" << "FOCUS" << "GUIDE" << "MOUNT"
            << "SCHEDULER" << "OBSERVATORY";

    m_EkosInterface->callWithArgumentList(QDBus::AutoDetect, "setEkosLoggingEnabled", QVariantList () << "FILE" << enabled);
    m_EkosInterface->callWithArgumentList(QDBus::AutoDetect, "setEkosLoggingEnabled", QVariantList () << "VERBOSE" << enabled);

    bool indiRequired = false;
    for (auto &oneButton : ui->modulesButtonGroup->buttons())
        indiRequired |= oneButton->text() != "FITS" && oneButton->isChecked();

    const bool enableINDI = enabled && indiRequired;
    m_EkosInterface->callWithArgumentList(QDBus::AutoDetect, "setEkosLoggingEnabled", QVariantList () << "INDI" << enableINDI);

    const bool enableFITS = enabled && ui->FITSCheck->isChecked();
    m_EkosInterface->callWithArgumentList(QDBus::AutoDetect, "setEkosLoggingEnabled", QVariantList () << "FITS" << enableFITS);

    const bool enableCapture = enabled && ui->captureCheck->isChecked();
    m_EkosInterface->callWithArgumentList(QDBus::AutoDetect, "setEkosLoggingEnabled",
                                          QVariantList () << "CAPTURE" << enableCapture);

    const bool enableFocus = enabled && ui->focusCheck->isChecked();
    m_EkosInterface->callWithArgumentList(QDBus::AutoDetect, "setEkosLoggingEnabled",
                                          QVariantList () << "FOCUS" << enableFocus);

    const bool enableGuide = enabled && ui->guideCheck->isChecked();
    m_EkosInterface->callWithArgumentList(QDBus::AutoDetect, "setEkosLoggingEnabled",
                                          QVariantList () << "GUIDE" << enableGuide);

    const bool enableMount = enabled && ui->mountCheck->isChecked();
    m_EkosInterface->callWithArgumentList(QDBus::AutoDetect, "setEkosLoggingEnabled",
                                          QVariantList () << "MOUNT" << enableMount);

    const bool enableScheduler = enabled && ui->schedulerCheck->isChecked();
    m_EkosInterface->callWithArgumentList(QDBus::AutoDetect, "setEkosLoggingEnabled",
                                          QVariantList () << "SCHEDULER" << enableScheduler);

    const bool enableAlign = enabled && ui->alignCheck->isChecked();
    m_EkosInterface->callWithArgumentList(QDBus::AutoDetect, "setEkosLoggingEnabled",
                                          QVariantList () << "ALIGNMENT" << enableAlign);



}
