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

#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <interfaces/iprojectcontroller.h>

#include <kconfiggroup.h>
#include <kfileitem.h>

AllProfilesModel::AllProfilesModel(UploadPlugin* plugin, QObject *parent)
    : QAbstractListModel(parent), m_plugin(plugin)
{
}

AllProfilesModel::~AllProfilesModel()
{
}

QVariant AllProfilesModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid() || index.parent().isValid()) return QVariant();
    int rowOffset = 0;
    Q_FOREACH (UploadProfileModel* model, m_sourceModels) {
        int rowCount = model->rowCount(index.parent());
        if (index.row() - rowOffset < rowCount) {
            QVariant ret =  model->data(model->index(index.row() - rowOffset, index.column()), role);
            if (role == Qt::DisplayRole) {
                ret = model->project()->name() + ": " + ret.toString();
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

void AllProfilesModel::addModel(UploadProfileModel* model)
{
    beginResetModel();
    
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

    m_sourceModels.append(model);
    endResetModel();
}

void AllProfilesModel::removeModel(UploadProfileModel* model)
{
    beginResetModel();
    m_sourceModels.removeAt(m_sourceModels.indexOf(model));
    endResetModel();
}

void AllProfilesModel::sourceReset()
{
    beginResetModel();
    endResetModel();
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

UploadProfileItem* AllProfilesModel::uploadItem(const QModelIndex& index) const
{
    if (!index.isValid() || index.parent().isValid()) return nullptr;
    int rowOffset = 0;
    Q_FOREACH (UploadProfileModel* model, m_sourceModels) {
        int rowCount = model->rowCount(index.parent());
        if (index.row() - rowOffset < rowCount) {
            return model->uploadItem(model->index(index.row() - rowOffset, index.column()));
        }
        rowOffset += rowCount;
    }
    return nullptr;
}

UploadProfileItem* AllProfilesModel::uploadItem(int row, int column) const
{
    return uploadItem(index(row, column));
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
