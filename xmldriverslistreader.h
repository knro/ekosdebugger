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

#pragma once
#include <QList>
#include <QXmlStreamReader>
#include <QDebug>

class Group
{
    public:
        QString group() const
        {
            return m_group;
        }
        QList<QStringList> devices() const
        {
            return m_devices;
        }
        //    QString instructions() const {return m_instructions;}
        void setGroup(const QString &group)
        {
            this->m_group = group;
        }
        void setDevices(const QList<QStringList> &devicesList)
        {
            this->m_devices = devicesList;
        }
        QString m_group;
        QList<QStringList> m_devices;
};

class DriversList
{
    public:
        void addGroup(Group* group)
        {
            this->m_groups.append(group);
        };
        void print() const
        {
            foreach(auto &x, m_groups)
                qDebug() << x->m_devices;
        };
        QList<Group*> m_groups;

};

class XmlDriversListReader
{
    public:
        XmlDriversListReader(DriversList* driverslist);
        bool read(QIODevice *device);
        QString errorString() const
        {
            return reader.errorString();
        }
    private:
        QXmlStreamReader reader;
        DriversList* m_driverslist;

        void readDriversList();
        void readGroup();
        void readGroupName(Group *group);
        void readDeviceGroup(Group *group);
        QString readDeviceExec();
};
