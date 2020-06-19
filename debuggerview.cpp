#include "debuggerview.h"
#include "./ui_debuggerview.h"

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
