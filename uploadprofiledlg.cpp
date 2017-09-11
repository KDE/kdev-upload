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
#include <KLocalizedString>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <qglobal.h>

#include <kprotocolmanager.h>
#include <kprotocolinfo.h>
#include <kio/statjob.h>
#include <KJobWidgets>
#include <kmessagebox.h>

#include "ui_uploadprofiledlg.h"
#include "uploadprofileitem.h"

UploadProfileDlg::UploadProfileDlg(QWidget *parent)
    : QDialog (parent)
{
    setWindowTitle(i18n("Upload Profile"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QWidget* widget = new QWidget(this);
    m_ui = new Ui::UploadProfileDlg();
    m_ui->setupUi(widget);

    m_ui->browseButtonLocal->setIcon(QIcon::fromTheme("document-open"));
    connect(m_ui->browseButtonLocal, SIGNAL(clicked()), this, SLOT(browseLocal()));
    m_ui->browseButton->setIcon(QIcon::fromTheme("document-open"));
    connect(m_ui->browseButton, SIGNAL(clicked()), this, SLOT(browse()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotAcceptButtonClicked()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    mainLayout->addWidget(widget);
    mainLayout->addWidget(buttonBox);

    QStringList protocols = KProtocolInfo::protocols();
    protocols.sort();
    Q_FOREACH (QString p, protocols) {
        QUrl u;
        u.setScheme(p);
        if (KProtocolManager::supportsWriting(u) && KProtocolManager::supportsMakeDir(u)
            && KProtocolManager::supportsDeleting(u)) {
            m_ui->comboProtocol->addItem(p);
        }
    }
}

UploadProfileDlg::~UploadProfileDlg()
{
    delete m_ui;
}

int UploadProfileDlg::editProfile(UploadProfileItem* item)
{
    m_ui->lineProfileName->setText(item->text());
    m_ui->defaultProfile->setChecked(item->isDefault());
    m_ui->lineLocalPath->setText(item->localUrl().toString());
    updateUrl(item->url());

    int result = exec();
    if (result == QDialog::Accepted) {
        item->setText(m_ui->lineProfileName->text());
        item->setUrl(currentUrl());
        QUrl localUrl = QUrl(m_ui->lineLocalPath->text());
        item->setLocalUrl(localUrl);
        item->setDefault(m_ui->defaultProfile->checkState() == Qt::Checked);
    }
    return result;
}

QUrl UploadProfileDlg::currentUrl()
{
    QUrl url;
    url.setHost(m_ui->lineHost->text());
    url.setUserName(m_ui->lineUser->text());
    url.setPath(m_ui->linePath->text());
    if (m_ui->port->text().toInt() > 0) url.setPort(m_ui->port->text().toInt());
    url.setScheme(m_ui->comboProtocol->currentText());
    return url;
}

void UploadProfileDlg::updateUrl(const QUrl& url)
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
    QUrl chosenDir = QFileDialog::getExistingDirectoryUrl(this, QString(), currentUrl());

    if(chosenDir.isValid()) {
        updateUrl(chosenDir);
    }
}

void UploadProfileDlg::browseLocal()
{
    QUrl chosenDir = QFileDialog::getExistingDirectoryUrl(this, QString(), m_ui->lineLocalPath->text());

    if(chosenDir.isValid()) {
        m_ui->lineLocalPath->setText(chosenDir.path());
    }
}

void UploadProfileDlg::slotAcceptButtonClicked()
{
    KIO::StatJob* statJob = KIO::stat(currentUrl());
    statJob->setSide(KIO::StatJob::DestinationSide);
    KJobWidgets::setWindow(statJob, this);
    bool dirExists = statJob->exec();
    if (!dirExists) {
        KMessageBox::sorry(this, i18n("The specified URL does not exist."));
        return;
    }
    
    //TODO: check if local dir is subpath of project dir
    QString selectedLocalPath = m_ui->lineLocalPath->text();
    if(!QDir(selectedLocalPath).exists()) {
        KMessageBox::sorry(this, i18n("The specified local directory does not exist."));
        return;
    }
    
    QDialog::accept();
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
