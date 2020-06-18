#ifndef DEBUGGERVIEW_H
#define DEBUGGERVIEW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class DebuggerView; }
QT_END_NAMESPACE

class DebuggerView : public QMainWindow
{
    Q_OBJECT

public:
    DebuggerView(QWidget *parent = nullptr);
    ~DebuggerView();

private:
    Ui::DebuggerView *ui;
};
#endif // DEBUGGERVIEW_H
