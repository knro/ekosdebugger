#include "debuggerview.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Ikarus Technologies");
    QCoreApplication::setOrganizationDomain("ikarustech.com");
    QCoreApplication::setApplicationName("Ekos Debugger");

    DebuggerView w;
    w.show();
    return a.exec();
}
