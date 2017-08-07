/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef UPLOADDIALOG_H
#define UPLOADDIALOG_H

#include <QDialog>
#include <QModelIndex>

class QAbstractButton;
class QProgressDialog;
class QMenu;
class KJob;
namespace KDevelop {
    class IProject;
    class ProjectBaseItem;
}
class UploadProjectModel;
class UploadProfileModel;
class UploadProfileDlg;
class UploadPlugin;
namespace Ui {
    class UploadDialog;
}

/**
 * Dialog where the user can select the files to upload
 */
class UploadDialog : public QDialog
{
    Q_OBJECT

public:
    UploadDialog(KDevelop::IProject* project, UploadPlugin* plugin, QWidget *parent = nullptr);
    ~UploadDialog() override;

    /**
     * Set the root-item, only items below this are displayed
     */
    void setRootItem(KDevelop::ProjectBaseItem* item);

    /**
     * Set the UploadProfileModel
     */
    void setProfileModel(UploadProfileModel* model);

private Q_SLOTS:
    /**
     * Starts the upload
     */
    void startUpload();

    /**
     * Current profile changed, sets the new profile to the model
     */
    void profileChanged(int index);

    /**
     * Opens the  UploadProfileDlg to edit the current profile
     */
    void modifyProfile();

    /**
     * Called when the upload successfully finished, closes the UploadDialog
     */
    void uploadFinished();

protected:
    /**
     * Event-filter for tree context-menu.
     */
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Ui::UploadDialog* m_ui;
    KDevelop::IProject* m_project;
    UploadProjectModel* m_uploadProjectModel;
    UploadProfileModel* m_profileModel;
    UploadProfileDlg* m_editProfileDlg;
    UploadPlugin* m_plugin;
    QMenu* m_treeContextMenu;
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
