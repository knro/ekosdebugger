#ifndef DEBUGGERVIEW_H
#define DEBUGGERVIEW_H

#include <QMainWindow>
#include <QProcess>
#include <QPointer>
#include <QJsonObject>
#include <QList>

#include <memory>

#include "profileinfo.h"

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
        void copyKStarsDebugLog();
        void copyKStarsAppLog();
        void processINDIOutput();
        void processINDIError();
        void copyINDIDebugLog();
        void copyINDIAppLog();


    private:
        QPointer<QProcess> m_KStarsProcess;
        QPointer<QProcess> m_INDIProcess;
        Ui::DebuggerView *ui;
        QList<std::shared_ptr<ProfileInfo>> profiles;
        void startKStars();
        void stopKStars();
        void startINDI();
        void stopINDI();
        void loadProfiles();
        void loadDrivers();

};
#endif // DEBUGGERVIEW_H
