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
class KUrl;

class UploadProjectModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    UploadProjectModel( KDevelop::IProject* project, QObject *parent = 0 );
    virtual ~UploadProjectModel();

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
    Qt::ItemFlags flags ( const QModelIndex & index ) const;

    bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;

    void setProfileConfigGroup(const KConfigGroup& group);
    KConfigGroup profileConfigGroup() const;

    KDevelop::ProjectModel* projectModel() const;
    KDevelop::ProjectBaseItem* item(const QModelIndex& index) const;

    QModelIndex nextRecursionIndex(const QModelIndex& current, const QModelIndex& root = QModelIndex()) const;

    void setRootItem(KDevelop::ProjectBaseItem* item);

private:
    KDevelop::IProject* m_project;
    KSharedConfig::Ptr m_projectConfig;
    KConfigGroup m_profileConfigGroup;
    QMap<KUrl, Qt::CheckState> m_checkStates;
    KDevelop::ProjectBaseItem* m_rootItem;
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
