#include "debuggerview.h"
#include "./ui_debuggerview.h"

#include <QDebug>

#include "userdb.h"

DebuggerView::DebuggerView(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DebuggerView)
{
    ui->setupUi(this);
    ui->statusbar->showMessage("Welcome to Ekos Debugger!");

    connect(ui->startKStarsB, &QPushButton::clicked, this, &DebuggerView::startKStars);
    connect(ui->stopKStarsB, &QPushButton::clicked, this, &DebuggerView::stopKStars);
    connect(ui->copyKStarsDebugLogB, &QPushButton::clicked, this, &DebuggerView::copyKStarsDebugLog);
    connect(ui->copyKStarsAppLogB, &QPushButton::clicked, this, &DebuggerView::copyKStarsAppLog);
    connect(ui->startINDIB, &QPushButton::clicked, this, &DebuggerView::startINDI);
    connect(ui->stopINDIB, &QPushButton::clicked, this, &DebuggerView::stopINDI);
    connect(ui->copyINDIDebugLogB, &QPushButton::clicked, this, &DebuggerView::copyINDIDebugLog);
    connect(ui->copyINDIAppLogB, &QPushButton::clicked, this, &DebuggerView::copyINDIAppLog);

    loadProfiles();
}

DebuggerView::~DebuggerView()
{
    delete ui;
}

void DebuggerView::startKStars()
{
    m_KStarsProcess = new QProcess();
    QStringList args;
    args << "-batch" << "-ex" << "run" << "-ex" << "bt" << "kstars";
    ui->startKStarsB->setDisabled(true);
    ui->stopKStarsB->setDisabled(false);
    ui->statusbar->showMessage("Started KStars.");


    connect(m_KStarsProcess, &QProcess::readyReadStandardOutput, this, &DebuggerView::processKStarsOutput);
    connect(m_KStarsProcess, &QProcess::readyReadStandardError, this, &DebuggerView::processKStarsError);
    m_KStarsProcess->start("gdb", args);

}

void DebuggerView::stopKStars()
{
    m_KStarsProcess->terminate();
    ui->startKStarsB->setDisabled(false);
    ui->stopKStarsB->setDisabled(true);
    ui->statusbar->showMessage("Stopped KStars.");
}

void DebuggerView::processKStarsOutput()
{
    QString buffer = m_KStarsProcess->readAllStandardOutput();
    buffer = buffer.trimmed();
    ui->KStarsDebugLog->append(buffer);
}

void DebuggerView::processKStarsError()
{
    QString buffer = m_KStarsProcess->readAllStandardError();
    buffer = buffer.trimmed();
    ui->KStarsAppLog->append(buffer);
}

void DebuggerView::copyKStarsDebugLog()
{
    ui->KStarsDebugLog->selectAll();
    ui->KStarsDebugLog->copy();
    ui->statusbar->showMessage("Copied KStars Debug log.");
}

void DebuggerView::copyKStarsAppLog()
{
    ui->KStarsAppLog->selectAll();
    ui->KStarsAppLog->copy();
    ui->statusbar->showMessage("Copied KStars Application log.");
}

void DebuggerView::startINDI()
{
    m_INDIProcess = new QProcess();
    QStringList args;
    args << "-batch" << "-ex" << "run" << "-ex" << "set follow-fork-mode child" << "-ex" << "run" << "-ex" << "bt" << "--args"
         << "indiserver" << "-r" << "0" << "-v";
    //    gdb -batch -ex "set follow-fork-mode child" -ex "run" -ex "bt" --args indiserver -r 0 -v
    ui->startINDIB->setDisabled(true);
    ui->stopINDIB->setDisabled(false);
    ui->statusbar->showMessage("Started INDI.");


    connect(m_INDIProcess, &QProcess::readyReadStandardOutput, this, &DebuggerView::processINDIOutput);
    connect(m_INDIProcess, &QProcess::readyReadStandardError, this, &DebuggerView::processINDIError);
    m_INDIProcess->start("gdb", args);
}

void DebuggerView::stopINDI()
{
    m_INDIProcess->terminate();
    ui->startINDIB->setDisabled(false);
    ui->stopINDIB->setDisabled(true);
    ui->statusbar->showMessage("Stopped INDI.");
}

void DebuggerView::processINDIOutput()
{
    QString buffer = m_INDIProcess->readAllStandardOutput();
    buffer = buffer.trimmed();
    ui->INDIDebugLog->append(buffer);
}

void DebuggerView::processINDIError()
{
    QString buffer = m_INDIProcess->readAllStandardError();
    buffer = buffer.trimmed();
    ui->INDIAppLog->append(buffer);
}

void DebuggerView::copyINDIDebugLog()
{
    ui->INDIDebugLog->selectAll();
    ui->INDIDebugLog->copy();
    ui->statusbar->showMessage("Copied INDI Debug log.");
}

void DebuggerView::copyINDIAppLog()
{
    ui->INDIAppLog->selectAll();
    ui->INDIAppLog->copy();
    ui->statusbar->showMessage("Copied INDI Application log.");
}

//from manager.cpp
void DebuggerView::loadProfiles()
{
    profiles.clear();
    ProfileDB::GetAllProfiles(profiles);

    for (auto &pi : profiles)
    {
        qDebug() << pi->name;
        ui->profileCombo->addItem(pi->name);
    }


}

//from manager.cpp
//void DebuggerView::loadDrivers()
//{
//    foreach (DriverInfo * dv, DriverManager::Instance()->getDrivers())
//    {
//        if (dv->getDriverSource() != HOST_SOURCE)
//            driversList[dv->getLabel()] = dv;
//    }
//}
