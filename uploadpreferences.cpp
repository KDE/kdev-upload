/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "uploadpreferences.h"

#include <QVBoxLayout>
#include <QStandardItemModel>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <util/path.h>

#include "uploadprofilemodel.h"
#include "ui_uploadpreferences.h"
#include "uploadprofiledlg.h"
#include "uploadprofileitem.h"

using namespace KDevelop;

UploadPreferences::UploadPreferences( KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent )
    : ProjectConfigPage<UploadSettings>(plugin, options, parent)
{
    IProject* project = options.project;

    m_ui = new Ui::UploadPreferences();
    m_ui->setupUi(this);

    m_model = new UploadProfileModel();
    m_model->setProject(project);
    m_ui->profilesList->setModel(m_model);


    connect(m_model, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this, SLOT(changed()));
    connect(m_model, SIGNAL(rowsRemoved(QModelIndex, int, int)),
            this, SLOT(changed()));

    connect(m_ui->addProfileButton, SIGNAL(clicked()),
              this, SLOT(addProfile()));
    connect(m_ui->modifyProfileButton, SIGNAL(clicked()),
              this, SLOT(modifyProfile()));
    connect(m_ui->removeProfileButton, SIGNAL(clicked()),
              this, SLOT(removeProfile()));
    connect(m_ui->profilesList, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(modifyProfile()));

    m_dlg = new UploadProfileDlg(this);
}

UploadPreferences::~UploadPreferences( )
{
  delete m_ui;
}

void UploadPreferences::reset()
{
    ProjectConfigPage::reset();
}

void UploadPreferences::apply()
{
    m_model->submit();
    ProjectConfigPage::apply();
}

void UploadPreferences::defaults()
{
    ProjectConfigPage::defaults();
}

void UploadPreferences::addProfile()
{
    UploadProfileItem* i = new UploadProfileItem();
    if (m_model->rowCount() == 0) {
        i->setDefault(true);
    }
    m_model->appendRow(i);
    if (m_dlg->editProfile(i) == QDialog::Rejected) {
        m_model->removeRow(i->index().row());
    }
}

void UploadPreferences::removeProfile()
{
    Q_FOREACH(QModelIndex index, m_ui->profilesList->selectionModel()->selectedIndexes()) {
        m_model->removeRow(index.row());
    }
}

void UploadPreferences::modifyProfile()
{
    Q_FOREACH(QModelIndex index, m_ui->profilesList->selectionModel()->selectedIndexes()) {
        UploadProfileItem* i = m_model->uploadItem(index);
        if (i) {
            m_dlg->editProfile(i);
        }
    }
}

QString UploadPreferences::name() const
{
    return i18n("Upload");
}

QString UploadPreferences::fullName() const
{
    return i18n("Configure Upload settings");
}

QIcon UploadPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("go-up"));
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
