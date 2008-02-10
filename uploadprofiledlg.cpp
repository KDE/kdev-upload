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

#include "ui_uploadprofiledlg.h"
#include "uploadprofileitem.h"

UploadProfileDlg::UploadProfileDlg( QWidget *parent)
    : QDialog ( parent )
{
    m_ui = new Ui::UploadProfileDlg();
    m_ui->setupUi(this);
}

UploadProfileDlg::~UploadProfileDlg( )
{
    delete m_ui;
}

int UploadProfileDlg::editProfile(UploadProfileItem* item)
{
    KUrl url = item->url();

    m_ui->lineProfileName->setText(item->text());
    m_ui->lineHost->setText(url.host());
    m_ui->lineUser->setText(url.userName());
    m_ui->linePath->setText(url.path());
    if (url.port() > 0) {
        m_ui->port->setText(QString::number(url.port()));
    } else {
        m_ui->port->setText("");
    }
    QString def_p = url.scheme();
  
    QStringList protocols = KProtocolInfo::protocols();
    protocols.sort();
    Q_FOREACH (QString p, protocols)
    {
        KUrl u;
        u.setProtocol(p);
        if (KProtocolManager::supportsWriting(u) && KProtocolManager::supportsMakeDir(u)
            && KProtocolManager::supportsDeleting(u)) {
            m_ui->comboProtocol->addItem(p);
            if ( p == def_p )
                m_ui->comboProtocol->setCurrentIndex(m_ui->comboProtocol->count()-1 );
        }
    }

    m_ui->defaultProfile->setChecked(item->isDefault());

    int result = exec();
    if (result == QDialog::Accepted) {
        item->setText(m_ui->lineProfileName->text());
        url.setHost(m_ui->lineHost->text());
        url.setUserName(m_ui->lineUser->text());
        url.setPath(m_ui->linePath->text());
        if (m_ui->port->text().toInt() > 0) url.setPort(m_ui->port->text().toInt());
        url.setScheme(m_ui->comboProtocol->currentText());
        item->setUrl(url);
        item->setDefault(m_ui->defaultProfile->checkState() == Qt::Checked);
    }
    return result;
}

#include "uploadprofiledlg.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
