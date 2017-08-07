/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef UPLOADPROFILEMODEL_H
#define UPLOADPROFILEMODEL_H

#include <QStandardItemModel>

namespace KDevelop {
    class IProject;
}
class UploadProfileItem;

/**
 * Model for upload-profiles.
 */
class UploadProfileModel : public QStandardItemModel
{
    Q_OBJECT
public:
    UploadProfileModel(QObject* parent = nullptr);
    ~UploadProfileModel() override {}

    /**
     * Removes an upload item
     */
    bool removeRow(int row, const QModelIndex & parent = QModelIndex());

    /**
     * Convenience function that returns a casted profile-item for a row.
     */
    UploadProfileItem* uploadItem(int row, int column = 0) const;
    /**
     * Convenience function that returns a casted profile-item for an index.
     */
    UploadProfileItem* uploadItem(const QModelIndex& index) const;

    /**
     * Set the Project for this model
     */
    void setProject(KDevelop::IProject* project);
    /**
     * Returns the project used for this model
     */
    KDevelop::IProject* project();

    /**
     * Reverts all changes made to the model
     */
    void revert() override;

    /**
     * Saves all changes made to the model in the config
     */
    bool submit() override;

private:
    KDevelop::IProject* m_project; ///< the project
    QStringList m_deltedProfileNrs; ///< deleted profiles
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
