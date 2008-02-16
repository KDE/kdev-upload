/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "profilesfiletree.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>

#include <kdebug.h>
#include <kfiletreeview.h>
#include <klocale.h>

#include <core.h>
#include <idocument.h>
#include <idocumentcontroller.h>

#include "allprofilesmodel.h"
#include "uploadprofileitem.h"

ProfilesFileTree::ProfilesFileTree(UploadPlugin* plugin, QWidget *parent)
    : QWidget(parent), m_plugin(plugin)
{
    QVBoxLayout *l = new QVBoxLayout();
    setLayout(l);

    m_profilesModel = new AllProfilesModel(plugin);

    m_profilesCombo = new QComboBox;
    m_profilesCombo->setModel(m_profilesModel);
    l->addWidget(m_profilesCombo);
    connect(m_profilesCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(profileIndexChanged(int)));
    connect(m_profilesModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this, SLOT(dataChanged(QModelIndex, QModelIndex)));

    m_pleaseSelectLabel = new QLabel(i18n("Please choose an upload profile."));
    m_pleaseSelectLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    l->addWidget(m_pleaseSelectLabel);

    m_tree = new KFileTreeView();
    m_tree->setVisible(false);
    for (int i = 1; i < m_tree->model()->columnCount(); ++i) {
        m_tree->hideColumn(i);
    }
    m_tree->header()->hide();
    connect(m_tree, SIGNAL(activated(KUrl)),
            this, SLOT(openUrl(KUrl)));

    //additional to activate also connect doubleClicked
    //activated *should* be emitted on doubleclick - but for me it doesn't
    connect(m_tree, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(treeDoubleClicked()));

    l->addWidget(m_tree);
}
void ProfilesFileTree::profileIndexChanged(int index)
{
    if (index == -1) {
        m_tree->setVisible(false);
        m_pleaseSelectLabel->setVisible(true);
        return;
    }

    UploadProfileItem* item = m_profilesModel->uploadItem(
                            m_profilesModel->index(index, 0, QModelIndex()));
    if (item) {
        m_tree->setVisible(true);
        m_pleaseSelectLabel->setVisible(false);
        if (m_tree->rootUrl() != item->url()) {
            m_tree->setRootUrl(item->url());
        }
    } else {
        profileIndexChanged(-1);
    }
}

void ProfilesFileTree::treeDoubleClicked()
{
    if (m_tree->selectedUrl().isValid()) {
        openUrl(m_tree->selectedUrl());
    }
}

void ProfilesFileTree::openUrl(const KUrl& url)
{
    kDebug(24000) << "openUrl" << url;
    KDevelop::Core::self()->documentController()->openDocument(url);
}
void ProfilesFileTree::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    profileIndexChanged(m_profilesCombo->currentIndex());
}

#include "profilesfiletree.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
