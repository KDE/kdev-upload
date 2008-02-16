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

#include <ksharedconfig.h>

class UploadProfileItem;

/**
 * Model for upload-profiles.
 */
class UploadProfileModel : public QStandardItemModel
{
    Q_OBJECT
public:
    UploadProfileModel(QObject* parent = 0);
    virtual ~UploadProfileModel() {}

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
     * Set the ProjectConfig for this model
     */
    void setConfig(KSharedConfig::Ptr config);
    /**
     * Returns the project-config used for this model
     */
    KSharedConfig::Ptr config();

    /**
     * Reverts all changes made to the model
     */
    void revert();

    /**
     * Saves all changes made to the model in the config
     */
    bool submit();

private:
    KSharedConfig::Ptr m_config; ///< the project-config
    QStringList m_deltedProfileNrs; ///< deleted profiles
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
