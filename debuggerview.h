#ifndef DEBUGGERVIEW_H
#define DEBUGGERVIEW_H

#include <QMainWindow>
#include <QProcess>
#include <QPointer>

QT_BEGIN_NAMESPACE
namespace Ui
{
class DebuggerView;
}
QT_END_NAMESPACE

class DebuggerView : public QMainWindow
{
        Q_OBJECT

    public:
        DebuggerView(QWidget *parent = nullptr);
        ~DebuggerView();

    private slots:
        void processKStarsOutput();
        void processKStarsError();


    private:
        QPointer<QProcess> m_KStarsProcess;
        Ui::DebuggerView *ui;
        void startKStars();
        void stopKStars();

};
#endif // DEBUGGERVIEW_H
