#include "debuggerview.h"
#include "./ui_debuggerview.h"

DebuggerView::DebuggerView(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DebuggerView)
{
    ui->setupUi(this);
}

DebuggerView::~DebuggerView()
{
    delete ui;
}

