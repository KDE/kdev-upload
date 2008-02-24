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

    /**
     * Returns a UploadProfileItem for a row
     */
    UploadProfileItem* uploadItem(int row, int column = 0) const;

private Q_SLOTS:
    /**
     * When a project is opened this adds the UploadProfileModel of the project
     * to the sourceModels
     */
    void projectOpened(KDevelop::IProject* p);

    /**
     * When a project is closed this removes the UploadProfileModel of the project
     * from the sourceModels
     */
    void projectClosed(KDevelop::IProject* p);

    //translate signals from sourceModels:
    void sourceReset();
    void sourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void sourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    void sourceRowsInserted();
    void sourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void sourceRowsRemoved();

    /**
     * Called when upload-configuration has changed (the user opened the
     * project-configuration and modified profiles)
     * KSettings::Dispatcher::registerComponent is used for this.
     */
    void reloadSettings();

private:
    QMap<KDevelop::IProject*, UploadProfileModel*> m_sourceModels;
    UploadPlugin* m_plugin;
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
