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

class UploadDialog : public QDialog
{
    Q_OBJECT

public:
    UploadDialog(KDevelop::IProject* project, QWidget *parent = 0);
    virtual ~UploadDialog();

    void setRootItem(KDevelop::ProjectBaseItem* item);

private Q_SLOTS:
    void buttonClicked(QAbstractButton* button);
    void profileChanged(int index);
    void modifyProfile();

private:
    QModelIndex m_uploadIndex;

    Ui::UploadDialog* m_ui;
    KDevelop::IProject* m_project;
    UploadProjectModel* m_uploadProjectModel;
    UploadProfileModel* m_profileModel;
    UploadProfileDlg* m_editProfileDlg;

    QProgressDialog* m_progressDialog;
    int m_progressBytesDone;
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
