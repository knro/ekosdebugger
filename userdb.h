
#pragma once

#include "profileinfo.h"
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStringList>
#include <QVariantMap>
#include <QXmlStreamReader>

#include <memory>

class ProfileDB
{
    public:
        static void GetAllProfiles(QList<std::shared_ptr<ProfileInfo>> &profiles);
};
