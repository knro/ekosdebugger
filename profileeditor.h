/*  Profile Editor
    Copyright (C) 2016 Jasem Mutlaq <mutlaqja@ikarustech.com>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/

#pragma once

#include "oal/scope.h"
#include "ui_profileeditor.h"

#include <QDialog>
#include <QFrame>
#include <QString>
#include <QStringList>
#include <QPointer>
#include <QProgressDialog>
#include <QNetworkAccessManager>

class ProfileInfo;
class QStandardItemModel;
class DriverInfo;

class ProfileEditorUI : public QFrame, public Ui::ProfileEditorUI
{
        Q_OBJECT

    public:
        /** @short Constructor */
        explicit ProfileEditorUI(QWidget *parent);
};

class ProfileEditor : public QDialog
{
        Q_OBJECT
    public:
        /** @short Constructor */
        explicit ProfileEditor(QWidget *ks);

        /** @short Destructor */
        virtual ~ProfileEditor() override = default;

        void setPi(ProfileInfo *value);


        void loadDrivers();
        void loadScopeEquipment();

    private:
        ProfileInfo *pi { nullptr };

};
