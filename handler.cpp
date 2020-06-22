#include "handler.h"
#include <QXmlParseException>
#include <QDebug>

Handler::Handler()
{

}

bool Handler::fatalError (const QXmlParseException &exception)
{
    qWarning() << "Fatal error on line" << exception.lineNumber()
               << ", column" << exception.columnNumber() << ':'
               << exception.message();

    return false;
}
