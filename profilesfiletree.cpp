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
#include <QPushButton>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <QDesktopServices>
#include "kdevuploaddebug.h"

#include <KLocalizedString>
#include <KDirOperator>
#include <KFileWidget>
#include <KActionCollection>
#include <kfileitem.h>

#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>

#include "allprofilesmodel.h"
#include "uploadprofileitem.h"
#include "uploadprofiledlg.h"

ProfilesFileTree::ProfilesFileTree(UploadPlugin* plugin, QWidget *parent)
    : QWidget(parent), m_plugin(plugin), m_editProfileDlg(nullptr)
{
    QVBoxLayout *l = new QVBoxLayout();
    setLayout(l);

    QHBoxLayout *hl = new QHBoxLayout();
    l->addLayout(hl);

    m_profilesCombo = new QComboBox;
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(1);
    m_profilesCombo->setSizePolicy(sizePolicy);
    hl->addWidget(m_profilesCombo);

    connect(m_profilesCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(profileIndexChanged(int)));

    QPushButton* editButton = new QPushButton("...");    
    hl->addWidget(editButton);

    connect(editButton, SIGNAL(clicked()), this, SLOT(modifyProfile()));

    m_pleaseSelectLabel = new QLabel(i18n("Please choose an upload profile."));
    m_pleaseSelectLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    l->addWidget(m_pleaseSelectLabel);


    m_tree = new KDirOperator();
    m_tree->setView(KFile::Tree);
    connect(m_tree, SIGNAL(fileSelected(const KFileItem &)),
           SLOT(fileSelected(KFileItem)));
    connect(m_tree, SIGNAL(urlEntered()), SLOT(urlEntered()), Qt::QueuedConnection);
    connect(m_tree, SIGNAL(contextMenuAboutToShow(KFileItem,QMenu*)), SLOT(contextMenuAboutToShow(KFileItem,QMenu*)));

    QAction* a = new QAction(i18n("&Copy URL"), this);
    connect(a, SIGNAL(triggered(bool)), SLOT(copyUrl()));
    m_tree->actionCollection()->addAction("copyUrl", a);
    a = new QAction(i18n("&Open URL"), this);
    connect(a, SIGNAL(triggered(bool)), SLOT(browseUrl()));
    m_tree->actionCollection()->addAction("browse", a);

    l->addWidget(m_tree);

}

void ProfilesFileTree::browseUrl()
{
    QUrl url;
    if (m_tree->selectedItems().isEmpty()) {
        url = m_tree->url();
    } else {
        url = m_tree->selectedItems().first().url();
    }
    QDesktopServices::openUrl(url);
}

void ProfilesFileTree::copyUrl()
{
    QUrl url;
    if (m_tree->selectedItems().isEmpty()) {
        url = m_tree->url();
    } else {
        url = m_tree->selectedItems().first().url();
    }
    QApplication::clipboard()->setText(url.url());
}


void ProfilesFileTree::contextMenuAboutToShow(KFileItem item, QMenu* menu)
{
    m_tree->setupMenu(KDirOperator::FileActions);
    menu->addSeparator();
    if (item.isDir()) {
        menu->addAction(m_tree->actionCollection()->action("browse"));
    }
    menu->addAction(m_tree->actionCollection()->action("copyUrl"));
}


void ProfilesFileTree::urlEntered()
{
    profileIndexChanged(m_profilesCombo->currentIndex());
}

void ProfilesFileTree::fileSelected(const KFileItem& item)
{
    openUrl(item.url());
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
        qCDebug(KDEVUPLOAD) << "item->url()" << item->url();
        m_tree->setVisible(true);
        m_pleaseSelectLabel->setVisible(false);
        qCDebug(KDEVUPLOAD) << "m_tree->url()" << m_tree->url();
        if (m_tree->url() != item->url()) {
            m_tree->setUrl(item->url(), true);
        }
    } else {
        profileIndexChanged(-1);
    }
}

void ProfilesFileTree::openUrl(const QUrl& url)
{
    qCDebug(KDEVUPLOAD) << "openUrl" << url;
    KDevelop::ICore::self()->documentController()->openDocument(url);
}
void ProfilesFileTree::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    profileIndexChanged(m_profilesCombo->currentIndex());
}

void ProfilesFileTree::modifyProfile()
{
    UploadProfileItem* i = m_profilesModel->uploadItem(m_profilesCombo->currentIndex());
    if (i) {
        if (!m_editProfileDlg) {
            m_editProfileDlg = new UploadProfileDlg(this);
        }
        if (m_editProfileDlg->editProfile(i) == QDialog::Accepted) {
            i->model()->submit();
        }
    }
}

void ProfilesFileTree::setModel(AllProfilesModel* model)
{
    m_profilesModel = model;

    m_profilesCombo->setModel(m_profilesModel);
    connect(m_profilesModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this, SLOT(dataChanged(QModelIndex, QModelIndex)));
}
    
    

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
