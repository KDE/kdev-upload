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

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kurl.h>
#include <kdebug.h>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include "uploadprofilemodel.h"
#include "ui_uploadpreferences.h"
#include "uploadprofiledlg.h"
#include "uploadprofileitem.h"
#include "kdevuploadplugin.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(UploadPreferencesFactory, registerPlugin<UploadPreferences>(); )
K_EXPORT_PLUGIN(UploadPreferencesFactory("kcm_kdev_upload"))

UploadPreferences::UploadPreferences( QWidget *parent, const QVariantList &args )
    : KCModule( UploadPreferencesFactory::componentData(), parent, args )
{
    IProject* project = 0;
    Q_FOREACH (IProject* p, KDevelop::ICore::self()->projectController()->projects()) {
        if (p->projectFileUrl() == args.at(1).toString()) {
            project = p;
            break;
        }
    }
    Q_ASSERT(project);


    setButtons(Help | Apply);

    QVBoxLayout * l = new QVBoxLayout( this );
    setLayout(l);
    QWidget* w = new QWidget();
    l->addWidget(w);

    m_ui = new Ui::UploadPreferences();
    m_ui->setupUi(w);

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

void UploadPreferences::save()
{
    m_model->submit();
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

#include "uploadpreferences.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
