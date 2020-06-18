#include "debuggerview.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DebuggerView w;
    w.show();
    return a.exec();
}
