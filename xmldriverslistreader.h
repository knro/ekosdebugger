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
        void print() const {
            foreach(auto &x,m_groups)
                qDebug()<<x->m_devices;
        };
        QList<Group*> m_groups;

};

class XmlDriversListReader
{
    public:
        XmlDriversListReader(DriversList* driverslist);
        bool read(QIODevice *device);
        QString errorString() const { return reader.errorString();}
    private:
        QXmlStreamReader reader;
        DriversList* m_driverslist;

        void readDriversList();
        void readGroup();
        void readGroupName(Group *group);
        void readDeviceGroup(Group *group);
        QString readDeviceExec();
};
