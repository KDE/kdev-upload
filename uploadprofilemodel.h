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

class UploadProfileModel : public QStandardItemModel
{
    Q_OBJECT
public:
    UploadProfileModel(QObject* parent = 0);
    virtual ~UploadProfileModel() {}

    bool removeRow(int row, const QModelIndex & parent = QModelIndex());

    UploadProfileItem* uploadItem(int row, int column = 0) const;
    UploadProfileItem* uploadItem(const QModelIndex& index) const;

    void setConfig(KSharedConfig::Ptr config);
    KSharedConfig::Ptr config();

    void revert();
    bool submit();

    QModelIndex nextRecursionIndex(const QModelIndex& current) const;

private:
    KSharedConfig::Ptr m_config;
    QStringList m_deltedProfileNrs;
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
