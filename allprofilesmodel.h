/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef ALLPROFILESMODEL_H
#define ALLPROFILESMODEL_H

#include <QAbstractListModel>
#include <QMap>

namespace KDevelop {
    class IProject;
}
class UploadProfileModel;
class UploadProfileItem;
class UploadPlugin;

/**
 * Model that lists upload-profiles of all open projects.
 *
 * Signals from the individual UploadProfileModels are translated
 * to the new index and emited (reset, dataChange, rowInsert*, rowRemove*).
 * This translation works only for single-row models - like UploadProfileModels.
 */
class AllProfilesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    AllProfilesModel(UploadPlugin* plugin, QObject *parent = 0);
    virtual ~AllProfilesModel();

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex & parent) const;

    /**
     * Returns a UploadProfileItem for a ModelIndex.
     */
    UploadProfileItem* uploadItem(const QModelIndex& index) const;

private Q_SLOTS:
    void projectOpened(KDevelop::IProject* p);
    void projectClosed(KDevelop::IProject* p);

    void sourceReset();
    void sourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void sourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    void sourceRowsInserted();
    void sourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void sourceRowsRemoved();

    void reloadSettings();

private:
    QMap<KDevelop::IProject*, UploadProfileModel*> m_sourceModels;
    UploadPlugin* m_plugin;
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
