#include "debuggerview.h"
#include "./ui_debuggerview.h"

DebuggerView::DebuggerView(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DebuggerView)
{
    ui->setupUi(this);

    connect(ui->startKStarsB, &QPushButton::clicked, this, &DebuggerView::startKStars);
    connect(ui->stopKStarsB, &QPushButton::clicked, this, &DebuggerView::stopKStars);
}

DebuggerView::~DebuggerView()
{
    delete ui;


}

void DebuggerView::startKStars()
{
    m_KStarsProcess = new QProcess();
    QStringList args;
    args << "-batch" << "-ex" << "run" << "kstars";

    connect(m_KStarsProcess, &QProcess::readyReadStandardOutput, this, &DebuggerView::processKStarsOutput);
    connect(m_KStarsProcess, &QProcess::readyReadStandardError, this, &DebuggerView::processKStarsError);
    m_KStarsProcess->start("gdb", args);

}

void DebuggerView::stopKStars()
{
    m_KStarsProcess->terminate();


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
