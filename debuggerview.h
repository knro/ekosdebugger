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

#include <QMainWindow>
#include <QProcess>
#include <QPointer>
#include <QJsonObject>
#include <QList>
#include <QTreeWidgetItem>
#include <QDir>
#include <QXmlInputSource>
#include <QXmlStreamReader>
#include <QFileDialog>
#include <QDateTime>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QMessageBox>

#include "handler.h"
#include "xmldriverslistreader.h"
#include "profileinfo.h"

#include <memory>

class ProfileInfo;
class QStandardItemModel;
class DriverInfo;

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

        enum
        {
            LOCAL_NAME_COLUMN = 0,
            LOCAL_STATUS_COLUMN,
            LOCAL_MODE_COLUMN,
            LOCAL_VERSION_COLUMN,
            LOCAL_PORT_COLUMN
        };
        enum
        {
            HOST_STATUS_COLUMN = 0,
            HOST_NAME_COLUMN,
            HOST_PORT_COLUMN
        };

        void setPi(ProfileInfo *value);
        QList<DriverInfo *> driversList;
        DriversList* driverslist = new DriversList;
        ProfileInfo * currProfile;
        bool readINDIHosts();
        bool readXMLDrivers();
        void readXMLDriverList(const QString &driverName);
        QString findDriverByLabel(const QString &label);
        bool checkDriverAvailability(const QString &driver);
        QStringList INDIArgs;
        QFileSystemWatcher watcher;
        QString KStarsLogFilePath;
        QString INDItimestamp = "";

    private slots:
        void processKStarsOutput();
        //        void processKStarsError();
        void copyKStarsDebugLog();
        //        void copyKStarsAppLog();
        void processINDIOutput();
        void processINDIError();
        void copyINDIDebugLog();
        void copyINDIAppLog();
        void createINDIArgs();
        void saveKStarsLogs();
        void saveINDILogs();
        void findLogFile(const QString &str);
        void clearKStarsDebugLog();
        void clearINDIDebugLog();
        void clearINDIAppLog();


    private:
        QPointer<QProcess> m_KStarsProcess;
        QPointer<QProcess> m_INDIProcess;
        QPointer<QProcess> m_zippingProcess;
        Ui::DebuggerView *ui;
        QList<std::shared_ptr<ProfileInfo>> profiles;
        void startKStars();
        void stopKStars();
        void startINDI();
        void stopINDI();
        void loadProfiles();
        void loadDriverCombo();
        void loadDrivers();

        QStringList driversStringList;
        ProfileInfo *pi { nullptr };
        QString getTooltip(DriverInfo *dv);
        void scanIP(const QString &ip);
        void clearAllRequests();

        QStandardItemModel *m_MountModel { nullptr };

};

