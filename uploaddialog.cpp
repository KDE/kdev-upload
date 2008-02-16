/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "uploaddialog.h"

#include <QPushButton>
#include <QHeaderView>
#include <QProgressDialog>

#include <kconfiggroup.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/jobuidelegate.h>
#include <kio/netaccess.h>
#include <kparts/mainwindow.h>

#include <core.h>
#include <iuicontroller.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <projectmodel.h>

#include "ui_uploaddialog.h"
#include "uploadprojectmodel.h"
#include "uploadprofilemodel.h"
#include "uploadprofileitem.h"
#include "uploadprofiledlg.h"
#include "uploadjob.h"
#include "kdevuploadplugin.h"

UploadDialog::UploadDialog(KDevelop::IProject* project, UploadPlugin* plugin, QWidget *parent)
    : QDialog(parent), m_project(project), m_editProfileDlg(0), m_plugin(plugin)
{
    m_ui = new Ui::UploadDialog();
    m_ui->setupUi(this);

    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("&Upload"));
    connect(m_ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
            this, SLOT(buttonClicked(QAbstractButton*)));

    m_uploadProjectModel = new UploadProjectModel(project);
    m_uploadProjectModel->setSourceModel(project->projectItem()->model());
    m_ui->projectTree->setModel(m_uploadProjectModel);
    m_ui->projectTree->header()->hide();

    m_profileModel = new UploadProfileModel();
    m_profileModel->setConfig(project->projectConfiguration());
    m_ui->profileCombobox->setModel(m_profileModel);

    connect(m_ui->profileCombobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(profileChanged(int)));

    m_ui->profileCombobox->setCurrentIndex(-1);
    for (int i = 0; i < m_profileModel->rowCount(); i++) {
        if (m_profileModel->uploadItem(i)->isDefault()) {
            m_ui->profileCombobox->setCurrentIndex(i);
            break;
        }
    }

    connect(m_ui->modifyProfileButton, SIGNAL(clicked()),
            this, SLOT(modifyProfile()));
}

UploadDialog::~UploadDialog()
{
    delete m_ui;
    delete m_editProfileDlg;
}

void UploadDialog::modifyProfile()
{
    UploadProfileItem* i = m_profileModel->uploadItem(m_ui->profileCombobox->currentIndex());
    if (i) {
        if (!m_editProfileDlg) {
            m_editProfileDlg = new UploadProfileDlg(this);
        }
        if (m_editProfileDlg->editProfile(i) == QDialog::Accepted) {
            m_profileModel->submit();
        }
    }
}

void UploadDialog::profileChanged(int index)
{
    UploadProfileItem* i = m_profileModel->uploadItem(index);
    if (i) {
        KConfigGroup c = i->profileConfigGroup();
        if (c.isValid()) {
            m_uploadProjectModel->setProfileConfigGroup(c);
            m_ui->projectTree->setEnabled(true);
            m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            return;
        }
    }
    m_ui->projectTree->setEnabled(false);
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void UploadDialog::buttonClicked(QAbstractButton* button)
{
    
    if (button == m_ui->buttonBox->button(QDialogButtonBox::Ok)) {
        if (m_ui->profileCombobox->currentIndex() == -1) {
            KMessageBox::sorry(this, i18n("Can't upload, no profile selected."));
            return;
        }
        QWidget* p = KDevelop::Core::self()->uiController()->activeMainWindow();
        UploadJob* job = new UploadJob(m_project, m_uploadProjectModel,
                            p);
        job->setOnlyMarkUploaded(m_ui->markUploadedCheckBox->checkState() == Qt::Checked);
        job->setOutputModel(m_plugin->outputModel());
        job->start();

        hide();
    }
}

void UploadDialog::setRootItem(KDevelop::ProjectBaseItem* item)
{
    m_uploadProjectModel->setRootItem(item);
    if (item) {
        QModelIndex i = m_uploadProjectModel->mapFromSource(item->index());
        while (i.isValid()) {
            m_ui->projectTree->expand(i);
            i = i.parent();
        }
    }
}

#include "uploaddialog.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
