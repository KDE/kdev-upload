/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "uploadprofiledlg.h"

#include <QListWidgetItem>

#include <kdebug.h>
#include <kprotocolmanager.h>
#include <kprotocolinfo.h>
#include <kurl.h>
#include <kdirselectdialog.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>

#include "ui_uploadprofiledlg.h"
#include "uploadprofileitem.h"

UploadProfileDlg::UploadProfileDlg(QWidget *parent)
    : KDialog (parent)
{
    QWidget* widget = new QWidget(this);
    m_ui = new Ui::UploadProfileDlg();
    m_ui->setupUi(widget);

    m_ui->browseButton->setIcon(KIcon("document-open"));
    connect(m_ui->browseButton, SIGNAL(clicked()), this, SLOT(browse()));

    QStringList protocols = KProtocolInfo::protocols();
    protocols.sort();
    Q_FOREACH (QString p, protocols) {
        KUrl u;
        u.setProtocol(p);
        if (KProtocolManager::supportsWriting(u) && KProtocolManager::supportsMakeDir(u)
            && KProtocolManager::supportsDeleting(u)) {
            m_ui->comboProtocol->addItem(p);
        }
    }

    setMainWidget(widget);
    setCaption(i18n("Upload Profile"));
    setButtons(KDialog::Close | KDialog::Ok);
}

UploadProfileDlg::~UploadProfileDlg()
{
    delete m_ui;
}

int UploadProfileDlg::editProfile(UploadProfileItem* item)
{
    m_ui->lineProfileName->setText(item->text());
    m_ui->defaultProfile->setChecked(item->isDefault());
    updateUrl(item->url());

    int result = exec();
    if (result == QDialog::Accepted) {
        item->setText(m_ui->lineProfileName->text());
        item->setUrl(currentUrl());
        item->setDefault(m_ui->defaultProfile->checkState() == Qt::Checked);
    }
    return result;
}

KUrl UploadProfileDlg::currentUrl()
{
    KUrl url;
    url.setHost(m_ui->lineHost->text());
    url.setUserName(m_ui->lineUser->text());
    url.setPath(m_ui->linePath->text());
    if (m_ui->port->text().toInt() > 0) url.setPort(m_ui->port->text().toInt());
    url.setScheme(m_ui->comboProtocol->currentText());
    return url;
}

void UploadProfileDlg::updateUrl(const KUrl& url)
{
    m_ui->lineHost->setText(url.host());
    m_ui->lineUser->setText(url.userName());
    m_ui->linePath->setText(url.path());
    if (url.port() > 0) {
        m_ui->port->setText(QString::number(url.port()));
    } else {
        m_ui->port->setText("");
    }
    int index = m_ui->comboProtocol->findData(url.scheme(), Qt::DisplayRole);
    m_ui->comboProtocol->setCurrentIndex(index);
}

void UploadProfileDlg::browse()
{
    KDirSelectDialog dialog(currentUrl(), false, this);
    if (dialog.exec() == QDialog::Accepted && dialog.url().isValid()) {
        updateUrl(dialog.url());
    }
}
void UploadProfileDlg::slotButtonClicked(int button) {
    if (button == KDialog::Ok) {
        if (!KIO::NetAccess::exists(currentUrl(), KIO::NetAccess::DestinationSide, this)) {
            KMessageBox::sorry(this, i18n("The specified url does not exist."));
            return;
        }
    }
    KDialog::slotButtonClicked(button);
}

#include "uploadprofiledlg.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
