/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef UPLOADPROJECTMODEL_H
#define UPLOADPROJECTMODEL_H

#include <QSortFilterProxyModel>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

namespace KDevelop {
    class IProject;
    class ProjectModel;
    class ProjectBaseItem;
}
class QUrl;

/**
 * ProxyModel that adds checkboxes for upload status to the ProjectModel.
 *
 * Reads out the modified time of files and compares it with the last upload time
 * to automatically select changed items.
 */
class UploadProjectModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    UploadProjectModel( KDevelop::IProject* project, QObject *parent = nullptr );
    ~UploadProjectModel() override;

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags ( const QModelIndex & index ) const override;

    bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const override;

    /**
     * Sets the profile KConfigGroup where the uploadtimes are stored.
     * called when user changes the active upload-profile.
     */
    void setProfileConfigGroup(const KConfigGroup& group);
    /**
     * Returns the KConfigGroup with the current upload-profile.
     */
    KConfigGroup profileConfigGroup() const;

    /**
     * Convenience function that returns the casted project-model.
     */
    KDevelop::ProjectModel* projectModel() const;

    /**
     * Convenience function that returns a casted project-item for an index.
     */
    KDevelop::ProjectBaseItem* item(const QModelIndex& index) const;

    /**
     * Helper-function to iterate recursive through the project-tree.
     * @param current the current index, start with QModelIndex()
     * @param root set to valid QModelIndex if a different root should be used
     * @return the next model-index in the iteration
     */
    QModelIndex nextRecursionIndex(const QModelIndex& current, const QModelIndex& root = QModelIndex()) const;

    void setRootItem(KDevelop::ProjectBaseItem* item);

    /**
     * Returns the name of the current Upload Profile (which is set through setProfileConfigGroup)
     */
    QString currentProfileName();

    /**
     * Returns the url of the current Upload Profile (which is set through setProfileConfigGroup)
     */
    QUrl currentProfileUrl();
    
    /**
     * Returns the local url of the current Upload Profile (which is set through setProfileConfigGroup)
     */
    QUrl currentProfileLocalUrl();

public Q_SLOTS:
    /**
     * Checks all items
     */
    void checkAll();

    /**
     * Checks all modified items
     */
    void checkModified();

    /**
     * Inverts the checkStatus of all items
     */
    void checkInvert();

private:
    KDevelop::IProject* m_project; ///< current project
    KConfigGroup m_profileConfigGroup; ///< KConfigGroup for active upload-profile
    QMap<QModelIndex, Qt::CheckState> m_checkStates; ///< holds the user-modified states of the checkboxes
    KDevelop::ProjectBaseItem* m_rootItem; ///< rootItem, tree is only displayed from here
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
