#pragma once
#include <QXmlSimpleReader>


class Handler : public QXmlDefaultHandler
{
public:
    Handler();
    bool fatalError (const QXmlParseException &exception) override;
};
