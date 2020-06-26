/*******************************************************************************
  Copyright(c) 2020 Ikarus Technologies. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*******************************************************************************/

#include "userdb.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>

#include <QStandardPaths>
#include <QDir>

void ProfileDB::GetAllProfiles(QList<std::shared_ptr<ProfileInfo>> &profiles)
{
    QSqlDatabase userdb = QSqlDatabase::addDatabase("QSQLITE", "userdb");

    const QString dbPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kstars/userdb.sqlite";
    userdb.setDatabaseName(dbPath);
    userdb.open();
    QSqlTableModel profile(nullptr, userdb);
    profile.setTable("profile");
    profile.select();

    for (int i = 0; i < profile.rowCount(); ++i)
    {
        QSqlRecord record = profile.record(i);

        int id       = record.value("id").toInt();
        QString name = record.value("name").toString();
        std::shared_ptr<ProfileInfo> pi(new ProfileInfo(id, name));

        // Add host and port
        pi->host = record.value("host").toString();
        pi->port = record.value("port").toInt();

        // City info
        pi->city     = record.value("city").toString();
        pi->province = record.value("province").toString();
        pi->country  = record.value("country").toString();

        pi->INDIWebManagerPort = record.value("indiwebmanagerport").toInt();
        pi->autoConnect        = (record.value("autoconnect").toInt() == 1);

        pi->indihub = record.value("indihub").toInt();

        pi->guidertype = record.value("guidertype").toInt();
        if (pi->guidertype != 0)
        {
            pi->guiderhost = record.value("guiderhost").toString();
            pi->guiderport = record.value("guiderport").toInt();
        }

        pi->primaryscope = record.value("primaryscope").toInt();
        pi->guidescope = record.value("guidescope").toInt();

        pi->remotedrivers = record.value("remotedrivers").toString();

        QSqlTableModel driver(nullptr, userdb);
        driver.setTable("driver");
        driver.setFilter("profile=" + QString::number(pi->id));
        driver.select();

        for (int i = 0; i < driver.rowCount(); ++i)
        {
            QSqlRecord record = driver.record(i);
            QString label     = record.value("label").toString();
            QString role      = record.value("role").toString();

            pi->drivers[role] = label;
        }
        driver.clear();

        profiles.append(pi);
    }

    profile.clear();
    userdb.close();
}

