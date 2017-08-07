/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "uploadprojectmodel.h"

#include <kconfiggroup.h>
#include <kfileitem.h>
#include <QDir>
#include "kdevuploaddebug.h"

#include <interfaces/iproject.h>
#include <util/path.h>

#include <project/projectmodel.h>

UploadProjectModel::UploadProjectModel(KDevelop::IProject* project, QObject *parent)
    : QSortFilterProxyModel(parent), m_project(project), m_rootItem(nullptr)
{
}

UploadProjectModel::~UploadProjectModel()
{
}

bool UploadProjectModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    KDevelop::ProjectBaseItem* item = projectModel()->itemFromIndex(index);
    if (!item) return false;
    if (item->project() != m_project) return false;
    if (!m_rootItem) return true;

    //is source a child of rootItem?
    QModelIndex i = index;
    while(i.isValid()) {
        if (m_rootItem->index() == i) return true;
        i = i.parent();
    }

    //is source a parent of rootItem?
    i = m_rootItem->index();
    while (i.isValid()) {
        if (index == i) return true;
        i = i.parent();
    }

    return false;
}

Qt::ItemFlags UploadProjectModel::flags(const QModelIndex & index) const
{
    Qt::ItemFlags ret = QSortFilterProxyModel::flags(index);
    ret |= Qt::ItemIsUserCheckable;
    ret &= ~Qt::ItemIsEditable;
    return ret;
}

QVariant UploadProjectModel::data(const QModelIndex & indx, int role) const
{
     if (indx.isValid() && role == Qt::CheckStateRole) {
        KDevelop::ProjectBaseItem* i = item(indx);
        if (i->file() && m_profileConfigGroup.isValid()) {
            if (m_checkStates.contains(indx)) {
                return m_checkStates.value(indx);
            } else {
                qCDebug(KDEVUPLOAD) << "project folder" << m_project->path().path() << "file" << i << i->file();
                qCDebug(KDEVUPLOAD) << "file url" << i->file()->path().path();
                QString url = m_project->path().relativePath(i->file()->path());
                qCDebug(KDEVUPLOAD) << "resulting url" << url;
                QDateTime uploadTime(m_profileConfigGroup.readEntry(url, QDateTime()));
                if (uploadTime.isValid()) {
                    KFileItem fileItem(i->file()->path().toUrl());
                    QDateTime modTime = fileItem.time(KFileItem::ModificationTime);
                    if (modTime > uploadTime) {
                        return Qt::Checked;
                    } else {
                        return Qt::Unchecked;
                    }
                } else {
                    return Qt::Checked;
                }
            }
        } else if (i->folder() && m_profileConfigGroup.isValid()) {
            if (!rowCount(indx)) {
                //empty folder - should be uploaded too
                if (m_checkStates.contains(indx)) {
                    return m_checkStates.value(indx);
                } else {
                    //don't check for ModificationTime as we do for files
                    QString url = m_project->path().relativePath(i->folder()->path());
                    QDateTime uploadTime(m_profileConfigGroup.readEntry(url, QDateTime()));
                    if (uploadTime.isValid()) {
                        return Qt::Unchecked;
                    } else {
                        return Qt::Checked;
                    }
                }
            }
            bool allChecked = true;
            bool noneChecked = true;
            for (int j = 0; j < rowCount(indx); j++) {
                Qt::CheckState s = static_cast<Qt::CheckState>(data(index(j, 0, indx), role).toInt());
                if (s == Qt::Checked) {
                    noneChecked = false;
                } else if (s == Qt::Unchecked) {
                    allChecked = false;
                } else {
                    return Qt::PartiallyChecked;
                }
            }
            if (allChecked) {
                return Qt::Checked;
            } else if (noneChecked) {
                return Qt::Unchecked;
            }
            return Qt::PartiallyChecked;
        } else {
            return QVariant();
        }
    }
    return QSortFilterProxyModel::data(indx, role);
}

bool UploadProjectModel::setData ( const QModelIndex & indx, const QVariant & value, int role)
{
    if (indx.isValid() && role == Qt::CheckStateRole) {
        KDevelop::ProjectBaseItem* i = item(indx);
        if (i->file()) {
            Qt::CheckState s = static_cast<Qt::CheckState>(value.toInt());
            m_checkStates.insert(indx, s);

            emit dataChanged(indx, indx);
            return true;
        } else if (i->folder()) {
            if (!rowCount(indx)) {
                //empty folder - should be uploaded too
                Qt::CheckState s = static_cast<Qt::CheckState>(value.toInt());
                m_checkStates.insert(indx, s);
                emit dataChanged(indx, indx);
            } else {
                //recursive check/uncheck
                QModelIndex i = indx;
                while((i = nextRecursionIndex(i, indx)).isValid()) {
                    setData(i, value, role);
                }
            }

            emit dataChanged(indx, indx);
            return true;
        }
    }
    return QSortFilterProxyModel::setData(indx, value, role);
}

void UploadProjectModel::setProfileConfigGroup(const KConfigGroup& group)
{
    beginResetModel();
    m_profileConfigGroup = group;
    m_checkStates.clear();
    endResetModel();
}

KConfigGroup UploadProjectModel::profileConfigGroup() const
{
    return m_profileConfigGroup;
}

KDevelop::ProjectModel* UploadProjectModel::projectModel() const
{
    return qobject_cast<KDevelop::ProjectModel*>(sourceModel());
}

KDevelop::ProjectBaseItem* UploadProjectModel::item(const QModelIndex& index) const
{
    return projectModel()->itemFromIndex(mapToSource(index));
}

QModelIndex UploadProjectModel::nextRecursionIndex(const QModelIndex& current, const QModelIndex& root) const
{
    QModelIndex ret;
    if (rowCount(current) > 0) {
        //firstChild
        return index(0, 0, current);
    } else if (current != root && current.isValid() &&
               current.row()+1 < rowCount(current.parent())) {
        //nextSibling
        return index(current.row()+1, 0, current.parent());
    }
    QModelIndex i = current;
    while (i.parent() != root && i.isValid() && i.parent().isValid()) {
        if (i.parent().row()+1 < rowCount(i.parent().parent())) {
            //parent+.nextSibling
            return index(i.parent().row()+1, 0, i.parent().parent());
        }
        i = i.parent();
    }

    //finished
    return QModelIndex();
}

void UploadProjectModel::setRootItem(KDevelop::ProjectBaseItem* item)
{
    beginResetModel();
    m_rootItem = item;
    endResetModel();
}

QString UploadProjectModel::currentProfileName()
{
    return m_profileConfigGroup.readEntry("name", QString());
}

QUrl UploadProjectModel::currentProfileUrl()
{
    return m_profileConfigGroup.readEntry("url", QUrl());
}

void UploadProjectModel::checkAll()
{
    setData(index(0, 0), Qt::Checked, Qt::CheckStateRole);
}

void UploadProjectModel::checkModified()
{
    QMapIterator<QModelIndex, Qt::CheckState> i(m_checkStates);
    m_checkStates.clear();
    while (i.hasNext()) {
        i.next();
        emit dataChanged(i.key(), i.key());
    }
}

void UploadProjectModel::checkInvert()
{
    QModelIndex index;
    while((index = nextRecursionIndex(index)).isValid()) {
        KDevelop::ProjectBaseItem* i = item(index);
        if (!(i->folder() && rowCount(index) > 0)) {
            //invert files and empty folders
            Qt::CheckState v = static_cast<Qt::CheckState>(data(index, Qt::CheckStateRole).toInt());
            if (v == Qt::Unchecked) v = Qt::Checked; else v = Qt::Unchecked;
            setData(index, v, Qt::CheckStateRole);
        }
    }
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
