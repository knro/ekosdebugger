bool DriverManager::readINDIHosts()
{
    QString indiFile("indihosts.xml");
    //QFile localeFile;
    QFile file;
    char errmsg[1024];
    char c;
    LilXML *xmlParser = newLilXML();
    XMLEle *root      = nullptr;
    XMLAtt *ap        = nullptr;
    QString hName, hHost, hPort;

    lastGroup = nullptr;
    file.setFileName(KSPaths::locate(QStandardPaths::GenericDataLocation, indiFile));
    if (file.fileName().isEmpty() || !file.open(QIODevice::ReadOnly))
    {
        delLilXML(xmlParser);
        return false;
    }

    while (file.getChar(&c))
    {
        root = readXMLEle(xmlParser, c, errmsg);

        if (root)
        {
            // Get host name
            ap = findXMLAtt(root, "name");
            if (!ap)
            {
                delLilXML(xmlParser);
                return false;
            }

            hName = QString(valuXMLAtt(ap));

            // Get host name
            ap = findXMLAtt(root, "hostname");

            if (!ap)
            {
                delLilXML(xmlParser);
                return false;
            }

            hHost = QString(valuXMLAtt(ap));

            ap = findXMLAtt(root, "port");

            if (!ap)
            {
                delLilXML(xmlParser);
                return false;
            }

            hPort = QString(valuXMLAtt(ap));

            DriverInfo *dv = new DriverInfo(hName);
            dv->setHostParameters(hHost, hPort);
            dv->setDriverSource(HOST_SOURCE);

            connect(dv, SIGNAL(deviceStateChanged(DriverInfo*)), this, SLOT(processDeviceStatus(DriverInfo*)));

            driversList.append(dv);

            QTreeWidgetItem *item = new QTreeWidgetItem(ui->clientTreeWidget, lastGroup);
            lastGroup             = item;
            item->setIcon(HOST_STATUS_COLUMN, ui->disconnected);
            item->setText(HOST_NAME_COLUMN, hName);
            item->setText(HOST_PORT_COLUMN, hPort);

            delXMLEle(root);
        }
        else if (errmsg[0])
        {
            qDebug() << errmsg;
            delLilXML(xmlParser);
            return false;
        }
    }

    delLilXML(xmlParser);

    return true;
}

bool DriverManager::readXMLDrivers()
{
    QDir indiDir;
    QString driverName;

    // This is the XML file shipped with KStars that contains all supported INDI drivers.
    /*QString indiDriversXML = KSPaths::locate(QStandardPaths::GenericDataLocation, "indidrivers.xml");
    if (indiDriversXML.isEmpty() == false)
        processXMLDriver(indiDriversXML);
    */

    processXMLDriver(QLatin1String(":/indidrivers.xml"));

    QString driversDir = Options::indiDriversDir();
#ifdef Q_OS_OSX
    if (Options::indiDriversAreInternal())
        driversDir = QCoreApplication::applicationDirPath() + "/../Resources/DriverSupport";
#endif

    if (indiDir.cd(driversDir) == false)
    {
        KSNotification::error(i18n("Unable to find INDI Drivers directory: %1\nPlease make sure to set the correct "
                                   "path in KStars configuration",
                                   driversDir));
        return false;
    }

    indiDir.setNameFilters(QStringList() << "indi_*.xml" << "drivers.xml");
    indiDir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList list = indiDir.entryInfoList();

    for (auto &fileInfo : list)
    {
        // libindi 0.7.1: Skip skeleton files
        if (fileInfo.fileName().endsWith(QLatin1String("_sk.xml")))
            continue;

        //        if (fileInfo.fileName() == "drivers.xml")
        //        {
        //            // Let first attempt to load the local version of drivers.xml
        //            driverName = KSPaths::writableLocation(QStandardPaths::GenericDataLocation) + "drivers.xml";

        //            // If found, we continue, otherwise, we load the system file
        //            if (driverName.isEmpty() == false && QFile(driverName).exists())
        //            {
        //                processXMLDriver(driverName);
        //                continue;
        //            }
        //        }

        processXMLDriver(fileInfo.absoluteFilePath());
    }

    return true;
}

void DriverManager::processXMLDriver(const QString &driverName)
{
    QFile file(driverName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KSNotification::error(i18n("Failed to open INDI Driver file: %1", driverName));
        return;
    }

    char errmsg[ERRMSG_SIZE];
    char c;
    LilXML *xmlParser = newLilXML();
    XMLEle *root      = nullptr;
    XMLEle *ep        = nullptr;

    if (driverName.endsWith(QLatin1String("drivers.xml")))
        driverSource = PRIMARY_XML;
    else
        driverSource = THIRD_PARTY_XML;

    while (file.getChar(&c))
    {
        root = readXMLEle(xmlParser, c, errmsg);

        if (root)
        {
            // If the XML file is using the INDI Library v1.3+ format
            if (!strcmp(tagXMLEle(root), "driversList"))
            {
                for (ep = nextXMLEle(root, 1); ep != nullptr; ep = nextXMLEle(root, 0))
                {
                    if (!buildDeviceGroup(ep, errmsg))
                        prXMLEle(stderr, ep, 0);
                }
            }
            // If using the older format
            else
            {
                if (!buildDeviceGroup(root, errmsg))
                    prXMLEle(stderr, root, 0);
            }

            delXMLEle(root);
        }
        else if (errmsg[0])
        {
            qCDebug(KSTARS_INDI) << QString(errmsg);
            delLilXML(xmlParser);
            return;
        }
    }

    delLilXML(xmlParser);
}

bool DriverManager::buildDeviceGroup(XMLEle *root, char errmsg[])
{
    XMLAtt *ap;
    XMLEle *ep;
    QString groupName;
    QTreeWidgetItem *group;
    DeviceFamily groupType = KSTARS_TELESCOPE;

    // avoid overflow
    if (strlen(tagXMLEle(root)) > 1024)
        return false;

    // Get device grouping name
    ap = findXMLAtt(root, "group");

    if (!ap)
    {
        snprintf(errmsg, ERRMSG_SIZE, "Tag %.64s does not have a group attribute", tagXMLEle(root));
        return false;
    }

    groupName = valuXMLAtt(ap);
    groupType = DeviceFamilyLabels.key(groupName);

#ifndef HAVE_CFITSIO
    // We do not create these groups if we don't have CFITSIO support
    if (groupType == KSTARS_CCD || groupType == KSTARS_VIDEO)
        return true;
#endif

    // Find if the group already exists
    QList<QTreeWidgetItem *> treeList = ui->localTreeWidget->findItems(groupName, Qt::MatchExactly);
    if (!treeList.isEmpty())
        group = treeList[0];
    else
        group = new QTreeWidgetItem(ui->localTreeWidget, lastGroup);

    group->setText(0, groupName);
    lastGroup = group;

    for (ep = nextXMLEle(root, 1); ep != nullptr; ep = nextXMLEle(root, 0))
    {
        if (!buildDriverElement(ep, group, groupType, errmsg))
            return false;
    }

    return true;
}

bool DriverManager::buildDriverElement(XMLEle *root, QTreeWidgetItem *DGroup, DeviceFamily groupType, char errmsg[])
{
    XMLAtt *ap;
    XMLEle *el;
    DriverInfo *dv;
    QString label;
    QString driver;
    QString version;
    // N.B. NOT an i18n string.
    QString manufacturer("Others");
    QString name;
    QString port;
    QString skel;
    QVariantMap vMap;
    //double focal_length(-1), aperture(-1);

    ap = findXMLAtt(root, "label");
    if (!ap)
    {
        snprintf(errmsg, ERRMSG_SIZE, "Tag %.64s does not have a label attribute", tagXMLEle(root));
        return false;
    }

    label = valuXMLAtt(ap);

    // Label is unique, so if we have the same label, we simply ignore
    if (findDriverByLabel(label) != nullptr)
        return true;

    ap = findXMLAtt(root, "manufacturer");
    if (ap)
        manufacturer = valuXMLAtt(ap);

    // Search for optional port attribute
    ap = findXMLAtt(root, "port");
    if (ap)
        port = valuXMLAtt(ap);

    // Search for skel file, if any
    // Search for optional port attribute
    ap = findXMLAtt(root, "skel");
    if (ap)
        skel = valuXMLAtt(ap);

    // Let's look for telescope-specific attributes: focal length and aperture
    //    ap = findXMLAtt(root, "focal_length");
    //    if (ap)
    //    {
    //        focal_length = QString(valuXMLAtt(ap)).toDouble();
    //        if (focal_length > 0)
    //            vMap.insert("TELESCOPE_FOCAL_LENGTH", focal_length);
    //    }

    // Find MDPD: Multiple Devices Per Driver
    ap = findXMLAtt(root, "mdpd");
    if (ap)
    {
        bool mdpd = false;
        mdpd      = (QString(valuXMLAtt(ap)) == QString("true")) ? true : false;
        vMap.insert("mdpd", mdpd);
    }

    //    ap = findXMLAtt(root, "aperture");
    //    if (ap)
    //    {
    //        aperture = QString(valuXMLAtt(ap)).toDouble();
    //        if (aperture > 0)
    //            vMap.insert("TELESCOPE_APERTURE", aperture);
    //    }

    el = findXMLEle(root, "driver");

    if (!el)
        return false;

    driver = pcdataXMLEle(el);

    ap = findXMLAtt(el, "name");
    if (!ap)
    {
        snprintf(errmsg, ERRMSG_SIZE, "Tag %.64s does not have a name attribute", tagXMLEle(el));
        return false;
    }

    name = valuXMLAtt(ap);

    el = findXMLEle(root, "version");

    if (!el)
        return false;

    version = pcdataXMLEle(el);
    bool versionOK = false;
    version.toDouble(&versionOK);
    if (versionOK == false)
        version = "1.0";

    bool driverIsAvailable = checkDriverAvailability(driver);

    vMap.insert("LOCALLY_AVAILABLE", driverIsAvailable);
    QIcon remoteIcon = QIcon::fromTheme("network-modem");

    QTreeWidgetItem *device = new QTreeWidgetItem(DGroup);

    device->setText(LOCAL_NAME_COLUMN, label);
    if (driverIsAvailable)
        device->setIcon(LOCAL_STATUS_COLUMN, ui->stopPix);
    else
        device->setIcon(LOCAL_STATUS_COLUMN, remoteIcon);
    device->setText(LOCAL_VERSION_COLUMN, version);
    device->setText(LOCAL_PORT_COLUMN, port);

    //if ((driverSource == PRIMARY_XML) && driversStringList.contains(driver) == false)
    if (groupType == KSTARS_TELESCOPE && driversStringList.contains(driver) == false)
        driversStringList.append(driver);

    dv = new DriverInfo(name);

    dv->setLabel(label);
    dv->setVersion(version);
    dv->setExecutable(driver);
    dv->setManufacturer(manufacturer);
    dv->setSkeletonFile(skel);
    dv->setType(groupType);
    dv->setDriverSource(driverSource);
    dv->setUserPort(port);
    dv->setAuxInfo(vMap);

    connect(dv, SIGNAL(deviceStateChanged(DriverInfo*)), this, SLOT(processDeviceStatus(DriverInfo*)));

    driversList.append(dv);

    return true;
}

bool DriverManager::checkDriverAvailability(const QString &driver)
{
    QString indiServerDir = Options::indiServer();
    if (Options::indiServerIsInternal())
        indiServerDir = QCoreApplication::applicationDirPath() + "/indi";
    else
        indiServerDir = QFileInfo(Options::indiServer()).dir().path();

    QFile driverFile(indiServerDir + '/' + driver);

    if (driverFile.exists() == false)
        return (!QStandardPaths::findExecutable(indiServerDir + '/' + driver).isEmpty());

    return true;
}

void DriverManager::updateCustomDrivers()
{
    for (const QVariantMap &oneDriver : m_CustomDrivers->customDrivers())
    {
        DriverInfo *dv = new DriverInfo(oneDriver["Name"].toString());
        dv->setLabel(oneDriver["Label"].toString());
        dv->setUniqueLabel(dv->getLabel());
        dv->setExecutable(oneDriver["Exec"].toString());
        dv->setVersion(oneDriver["Version"].toString());
        dv->setManufacturer(oneDriver["Manufacturer"].toString());
        dv->setType(DeviceFamilyLabels.key(oneDriver["Family"].toString()));
        dv->setDriverSource(CUSTOM_SOURCE);

        bool driverIsAvailable = checkDriverAvailability(oneDriver["Exec"].toString());
        QVariantMap vMap;
        vMap.insert("LOCALLY_AVAILABLE", driverIsAvailable);
        dv->setAuxInfo(vMap);

        driversList.append(dv);
    }
}
