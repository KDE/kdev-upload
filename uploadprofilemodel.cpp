/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "uploadprofilemodel.h"

#include <QUrl>

#include <KConfigGroup>
#include <ksettings/dispatcher.h>

#include <interfaces/iproject.h>

#include "uploadprofileitem.h"

UploadProfileModel::UploadProfileModel(QObject* parent)
    : QStandardItemModel(parent)
{
    KSettings::Dispatcher::registerComponent(QStringLiteral("kdevupload"), this, "revert");
}

bool UploadProfileModel::removeRow(int row, const QModelIndex & parent)
{
    UploadProfileItem* i = uploadItem(row);
    if (i && !i->profileNr().isEmpty()) {
        m_deltedProfileNrs << i->profileNr();
    }
    return QStandardItemModel::removeRow(row, parent);
}

UploadProfileItem* UploadProfileModel::uploadItem(int row, int column) const
{
    QStandardItem* i = item(row, column);
    if (i) {
        return dynamic_cast<UploadProfileItem*>(i);
    }
    return nullptr;
}
UploadProfileItem* UploadProfileModel::uploadItem(const QModelIndex& index) const
{
    QStandardItem* i = itemFromIndex(index);
    if (i) {
        return dynamic_cast<UploadProfileItem*>(i);
    }
    return nullptr;
}
void UploadProfileModel::setProject(KDevelop::IProject* project)
{
    m_project = project;
    revert();
}
KDevelop::IProject* UploadProfileModel::project()
{
    return m_project;
}

void UploadProfileModel::revert()
{
    KConfigGroup group = m_project->projectConfiguration()->group("Upload");
    QString defProfile = group.readEntry("default", QString());
    int row = 0;
    Q_FOREACH (QString g, group.groupList()) {
        if (g.startsWith("Profile")) {
            QUrl url = group.group(g).readEntry("url", QUrl());
            QString name = group.group(g).readEntry("name", QString());
            UploadProfileItem* i = uploadItem(row);
            if (!i) {
                i = new UploadProfileItem();
                insertRow(row, i);
            }
            i->setText(name);
            i->setUrl(url);
            i->setProfileNr(g.mid(7)); //group-name
            i->setDefault(i->profileNr() == defProfile);
            ++row;
        }
    }
    for (int i = row; i < rowCount(); ++i) {
        qDeleteAll(takeRow(i));
    }
}

bool UploadProfileModel::submit()
{
    KConfigGroup group = m_project->projectConfiguration()->group("Upload");
    Q_FOREACH (QString i, m_deltedProfileNrs) {
        group.group("Profile" + i).deleteGroup();
    }

    int maxProfileNr = 0;
    for (int i = 0; i < rowCount(); i++) {
        UploadProfileItem* item = uploadItem(i);
        if (item) {
            maxProfileNr = qMax(item->profileNr().toInt(), maxProfileNr);
        }
    }
    QString defaultProfileNr;
    for (int i = 0; i < rowCount(); i++) {
        UploadProfileItem* item = uploadItem(i);
        if (item) {
            if (item->profileNr().isEmpty()) {
                item->setProfileNr(QString::number(++maxProfileNr));
            }
            KConfigGroup profileGroup = group.group("Profile" + item->profileNr());
            profileGroup.writeEntry("url", item->url().toString());
            profileGroup.writeEntry("name", item->text());
            if (item->isDefault()) {
                defaultProfileNr = item->profileNr();
            }
        }
    }
    group.writeEntry("default", defaultProfileNr);
    return true;
}


// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
