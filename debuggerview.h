#ifndef DEBUGGERVIEW_H
#define DEBUGGERVIEW_H

#include <QMainWindow>
#include <QProcess>
#include <QPointer>
#include <QJsonObject>
#include <QList>
#include </usr/include/libindi/lilxml.h>
#include <QTreeWidgetItem>
#include "indicommon.h"
#include <QDir>
#include <QXmlInputSource>
#include "handler.h"
#include <QXmlStreamReader>
#include "xmldriverslistreader.h"

#include <memory>

#include "profileinfo.h"

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
        bool buildDeviceGroup(XMLEle *root, char errmsg[]);
        bool buildDriverElement(XMLEle *root, QTreeWidgetItem *DGroup, DeviceFamily groupType, char errmsg[]);
        bool checkDriverAvailability(const QString &driver);
        QStringList INDIArgs;


    private slots:
        void processKStarsOutput();
        void processKStarsError();
        void copyKStarsDebugLog();
        void copyKStarsAppLog();
        void processINDIOutput();
        void processINDIError();
        void copyINDIDebugLog();
        void copyINDIAppLog();
        void createINDIArgs();


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
        void loadDriverCombo();
        void loadDrivers();

        ServerMode connectionMode { SERVER_CLIENT };
        QTreeWidgetItem *lastGroup { nullptr };
        int currentPort;
        //DriverInfo::XMLSource xmlSource;
        DriverSource driverSource;
        QStringList driversStringList;
        //        QPointer<CustomDrivers> m_CustomDrivers;


        ProfileInfo *pi { nullptr };

        QString getTooltip(DriverInfo *dv);
        void scanIP(const QString &ip);
        void clearAllRequests();

        //        QList<OAL::Scope *> m_scopeList;
        QStandardItemModel *m_MountModel { nullptr };

};
#endif // DEBUGGERVIEW_H
