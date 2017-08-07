/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef PROFILESFILETREE_H
#define PROFILESFILETREE_H

#include <QWidget>
#include <kfileitem.h>

class QMenu;
class QModelIndex;
class QComboBox;
class QLabel;
class QUrl;

class KDirOperator;
class KFileItem;

class UploadPlugin;
class AllProfilesModel;
class UploadProfileDlg;

/**
 * The Profiles-File-Tree Dock
 *
 * A ComboBox to select the profile and a KFileTreeView below.
 */
class ProfilesFileTree : public QWidget
{
    Q_OBJECT

public:
    ProfilesFileTree( UploadPlugin* plugin, QWidget *parent = nullptr );

    /**
     * Sets the Model for this widget
     */
    void setModel(AllProfilesModel* model);

private Q_SLOTS:
    /**
     * Open the Url in the editor
     */
    void openUrl(const QUrl& url);

    /**
     * Slot called when current profile has changed, sets the new root-url for
     * the tree
     */
    void profileIndexChanged(int index);

    /**
     * Slot called when data of the profile has changed, sets the new root url
     * (if changed)
     */
    void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

    /**
     * Modify the current profile
     */
    void modifyProfile();

    void fileSelected(const KFileItem& item);
    void urlEntered();
    void contextMenuAboutToShow(KFileItem,QMenu*);
    void copyUrl();
    void browseUrl();

private:
    UploadPlugin* m_plugin;
    AllProfilesModel* m_profilesModel;

    QComboBox* m_profilesCombo;
    QLabel* m_pleaseSelectLabel;
    KDirOperator* m_tree;
    UploadProfileDlg* m_editProfileDlg;
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
