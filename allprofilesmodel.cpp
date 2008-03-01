/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "allprofilesmodel.h"
#include "uploadprofilemodel.h"
#include "kdevuploadplugin.h"

#include <iproject.h>
#include <projectmodel.h>
#include <iprojectcontroller.h>
#include <core.h>

#include <kconfiggroup.h>
#include <kfileitem.h>
#include <kdatetime.h>
#include <kdebug.h>
#include <ksettings/dispatcher.h>

AllProfilesModel::AllProfilesModel(UploadPlugin* plugin, QObject *parent)
    : QAbstractListModel(parent), m_plugin(plugin)
{
    connect(KDevelop::Core::self()->projectController(),
            SIGNAL(projectOpened(KDevelop::IProject*)),
                   this, SLOT(projectOpened(KDevelop::IProject*)));
    connect(KDevelop::Core::self()->projectController(),
            SIGNAL(projectClosed(KDevelop::IProject*)),
                   this, SLOT(projectClosed(KDevelop::IProject*)));

    KSettings::Dispatcher::registerComponent(KComponentData("KDevUpload"), this, "reloadSettings");
}

AllProfilesModel::~AllProfilesModel()
{
}

QVariant AllProfilesModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid() || index.parent().isValid()) return QVariant();
    int rowOffset = 0;
    QMapIterator<KDevelop::IProject*, UploadProfileModel*> it(m_sourceModels);
    while (it.hasNext()) {
        UploadProfileModel* model = it.next().value();
        int rowCount = model->rowCount(index.parent());
        if (index.row() - rowOffset < rowCount) {
            QVariant ret =  model->data(model->index(index.row() - rowOffset, index.column()), role);
            if (role == Qt::DisplayRole) {
                ret = it.key()->name() + ": " + ret.toString();
            }
            return ret;
        }
        rowOffset += rowCount;
    }
    return QVariant();
}

int AllProfilesModel::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid()) return 0;
    int ret = 0;
    Q_FOREACH (UploadProfileModel* model, m_sourceModels) {
        ret += model->rowCount();
    }
    return ret;
}

void AllProfilesModel::projectOpened(KDevelop::IProject* p)
{
    UploadProfileModel* model = new UploadProfileModel();
    model->setConfig(p->projectConfiguration());
    connect(model, SIGNAL(modelReset()), this, SLOT(sourceReset()));
    connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this, SLOT(sourceDataChanged(QModelIndex, QModelIndex)));
    connect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)),
            this, SLOT(sourceRowsAboutToBeInserted(QModelIndex, int, int)));
    connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SLOT(sourceRowsInserted()));
    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
            this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
    connect(model, SIGNAL(rowsRemoved(QModelIndex, int, int)),
            this, SLOT(sourceRowsRemoved()));
    void sourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end);

    m_sourceModels.insert(p, model);
    reset();
}

void AllProfilesModel::reloadSettings()
{
    QMapIterator<KDevelop::IProject*, UploadProfileModel*> i(m_sourceModels);
    while (i.hasNext()) {
        i.next();
        i.value()->setConfig(i.key()->projectConfiguration());
    }
}

void AllProfilesModel::sourceReset()
{
    reset();
}

void AllProfilesModel::sourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    if (topLeft.parent().isValid() || bottomRight.parent().isValid()) return;

    UploadProfileModel* sourceModel = qobject_cast<UploadProfileModel*>(sender());
    int rowOffset = 0;
    Q_FOREACH (UploadProfileModel* model, m_sourceModels) {
        if (model == sourceModel) {
            emit dataChanged(model->index(topLeft.row()+rowOffset, topLeft.column()),
                             model->index(bottomRight.row()+rowOffset, bottomRight.column()));
        }
        rowOffset += model->rowCount();
    }
}

void AllProfilesModel::sourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end)
{
    if (parent.isValid()) return;
    UploadProfileModel* sourceModel = qobject_cast<UploadProfileModel*>(sender());
    int rowOffset = 0;
    Q_FOREACH (UploadProfileModel* model, m_sourceModels) {
        if (model == sourceModel) {
            emit beginInsertRows(parent, start+rowOffset, end+rowOffset);
        }
        rowOffset += model->rowCount();
    }
}

void AllProfilesModel::sourceRowsInserted()
{
    emit endInsertRows();
}

void AllProfilesModel::sourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    if (parent.isValid()) return;
    UploadProfileModel* sourceModel = qobject_cast<UploadProfileModel*>(sender());
    int rowOffset = 0;
    Q_FOREACH (UploadProfileModel* model, m_sourceModels) {
        if (model == sourceModel) {
            emit beginRemoveRows(parent, start+rowOffset, end+rowOffset);
        }
        rowOffset += model->rowCount();
    }
}
void AllProfilesModel::sourceRowsRemoved()
{
    emit endRemoveRows();
}

void AllProfilesModel::projectClosed(KDevelop::IProject* p)
{
    m_sourceModels.remove(p);
    reset();
}

UploadProfileItem* AllProfilesModel::uploadItem(const QModelIndex& index) const
{
    if (!index.isValid() || index.parent().isValid()) return 0;
    int rowOffset = 0;
    Q_FOREACH (UploadProfileModel* model, m_sourceModels) {
        int rowCount = model->rowCount(index.parent());
        if (index.row() - rowOffset < rowCount) {
            return model->uploadItem(model->index(index.row() - rowOffset, index.column()));
        }
        rowOffset += rowCount;
    }
    return 0;
}

UploadProfileItem* AllProfilesModel::uploadItem(int row, int column) const
{
    return uploadItem(index(row, column));
}

#include "allprofilesmodel.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
