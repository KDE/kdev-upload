/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "uploadjob.h"

#include <QPushButton>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QtWidgets/QProgressDialog>
#include <QUrl>
#include <QDir>
#include "kdevuploaddebug.h"

#include <kconfiggroup.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/jobuidelegate.h>
#include <KLocalizedString>
#include <kjob.h>
#include <kjobwidgets.h>

#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include "uploadprojectmodel.h"

UploadJob::UploadJob(KDevelop::IProject* project, UploadProjectModel* model, QWidget *parent)
    : QObject(parent), m_project(project), m_uploadProjectModel(model),
      m_onlyMarkUploaded(false), m_quickUpload(false), m_outputModel(nullptr)
{
    m_progressDialog = new QProgressDialog();
    m_progressDialog->setWindowTitle(i18n("Uploading files"));
    m_progressDialog->setLabelText(i18n("Preparing..."));
    m_progressDialog->setModal(true);
}

UploadJob::~UploadJob()
{
    delete m_progressDialog;
}

void UploadJob::start()
{
    m_progressBytesDone = 0;
    m_progressDialog->setLabelText(i18n("Calculating size..."));
    m_progressDialog->setValue(0);
    m_progressDialog->show();

    int sumSize = 0;
    QModelIndex i;
    while((i = m_uploadProjectModel->nextRecursionIndex(i)).isValid()) {
        KDevelop::ProjectBaseItem* item = m_uploadProjectModel->item(i);
        Qt::CheckState checked = static_cast<Qt::CheckState>(m_uploadProjectModel
                                ->data(i, Qt::CheckStateRole).toInt());
        if (item->file() && checked != Qt::Unchecked) {
            KIO::UDSEntry entry;
            KIO::StatJob *statjob = KIO::stat(item->path().toUrl());
            KJobWidgets::setWindow(statjob, m_progressDialog);
            if (statjob->exec()) {
                entry = statjob->statResult();
                sumSize += entry.numberValue(KIO::UDSEntry::UDS_SIZE);
            }
        }
    }
    m_progressDialog->setMaximum(sumSize);

    m_uploadIndex = QModelIndex();
    uploadNext();
}

void UploadJob::uploadNext()
{
    if (m_progressDialog->wasCanceled()) return;

    m_uploadIndex = m_uploadProjectModel->nextRecursionIndex(m_uploadIndex);

    if (!m_uploadIndex.isValid()) {
        //last index reached - completed
        appendLog(i18n("Upload completed"));
        emit uploadFinished();
        delete this;
        return;
    }

    if (!m_uploadIndex.parent().isValid()) {
        //don't upload project root
        uploadNext();
        return;
    }

    KDevelop::ProjectBaseItem* item = m_uploadProjectModel->item(m_uploadIndex);

    Qt::CheckState checked = static_cast<Qt::CheckState>(m_uploadProjectModel
                            ->data(m_uploadIndex, Qt::CheckStateRole).toInt());

    KDevelop::Path url;
    QUrl localUrl = m_uploadProjectModel->currentProfileLocalUrl().adjusted(QUrl::StripTrailingSlash);

    KDevelop::Path localPath = KDevelop::Path(localUrl.path());
    
    if (item->folder()) {
        url = item->folder()->path();
    } else if (item->file()) {
        url = item->file()->path();
    }
    
    if(localPath.path().isEmpty()) {
        localPath = m_project->path();        
    }
    
    QString relativeUrl(localPath.relativePath(url));

    if (isQuickUpload() && checked == Qt::Unchecked) {
        appendLog(i18n("File was not modified for %1: %2",
                            m_uploadProjectModel->currentProfileName(),
                            relativeUrl));
    }

    if (!(item->file() || item->folder()) || checked == Qt::Unchecked) {
        uploadNext();
        return;
    }

    QUrl dest = m_uploadProjectModel->currentProfileUrl().adjusted(QUrl::StripTrailingSlash);
    dest.setPath(dest.path() + "/" + relativeUrl);
    KIO::Job* job = nullptr;

    if (m_onlyMarkUploaded) {
        appendLog(i18n("Marked as uploaded for %1: %2",
                            m_uploadProjectModel->currentProfileName(),
                            relativeUrl));
        m_uploadProjectModel->profileConfigGroup()
                .writeEntry(relativeUrl,
                            QDateTime::currentDateTime());
        uploadNext();
        return;
    } else if (item->file()) {
        appendLog(i18n("Uploading to %1: %2",
                            m_uploadProjectModel->currentProfileName(),
                            relativeUrl));
        qCDebug(KDEVUPLOAD) << "file_copy" << url.pathOrUrl() << dest;
        job = KIO::file_copy(url.toUrl(), dest, -1, KIO::Overwrite | KIO::HideProgressInfo);
        m_progressDialog->setLabelText(i18n("Uploading %1...", relativeUrl));
    } else if (item->folder()) {
        KIO::StatJob *statjob = KIO::stat(dest, KIO::StatJob::DestinationSide, 0);
        KJobWidgets::setWindow(statjob, m_progressDialog);
        if (statjob->exec()) {
            appendLog(i18n("Directory in %1 already exists: %2",
                                m_uploadProjectModel->currentProfileName(),
                                relativeUrl));
            m_uploadProjectModel->profileConfigGroup()
                    .writeEntry(relativeUrl,
                                QDateTime::currentDateTime());
            uploadNext();
            return;
        } else {
            appendLog(i18n("Creating directory in %1: %2",
                                m_uploadProjectModel->currentProfileName(),
                                relativeUrl));
            qCDebug(KDEVUPLOAD) << "mkdir" << dest;
            job = KIO::mkdir(dest);
        }
    } else {
        uploadNext();
        return;
    }

    KJobWidgets::setWindow(job, m_progressDialog);
    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(uploadResult(KJob*)));
    connect(job, SIGNAL(processedSize(KJob*, qulonglong)),
            this, SLOT(processedSize(KJob*, qulonglong)));
    connect(job, SIGNAL(infoMessage(KJob*, QString)),
            this, SLOT(uploadInfoMessage(KJob*, QString)));

    connect(m_progressDialog, SIGNAL(canceled()),
            this, SLOT(cancelClicked()));
    connect(m_progressDialog, SIGNAL(rejected()),
            job, SLOT(kill()));
    job->start();
}

void UploadJob::cancelClicked()
{
    appendLog(i18n("Upload canceled"));
    deleteLater();
}


void UploadJob::uploadResult(KJob* job)
{
    if (job->error()) {
        if (job->error() == KIO::ERR_USER_CANCELED) {
            cancelClicked();
            return;
        }
        appendLog(i18n("Upload error: %1", job->errorString()));
        job->uiDelegate()->showErrorMessage();
        deleteLater();
        return;
    }

    KDevelop::ProjectBaseItem* item = m_uploadProjectModel->item(m_uploadIndex);
    QUrl url;
    if (item->file()) {
        url = item->file()->path().toUrl();
    } else if (item->folder()) {
        url = item->folder()->path().toUrl();
    }
    m_uploadProjectModel->profileConfigGroup()
        .writeEntry(m_project->path().relativePath(KDevelop::Path(url)), QDateTime::currentDateTime());
    m_uploadProjectModel->profileConfigGroup().sync();

    KIO::UDSEntry entry;
    KIO::StatJob *statjob = KIO::stat(url, nullptr);
    KJobWidgets::setWindow(statjob, m_progressDialog);
    if (statjob->exec()) {
        entry = statjob->statResult();
        m_progressBytesDone += entry.numberValue(KIO::UDSEntry::UDS_SIZE);
    }
    m_progressDialog->setValue(m_progressBytesDone);

    uploadNext();
}

void UploadJob::processedSize(KJob*, qulonglong size)
{
    m_progressDialog->setValue(m_progressBytesDone + size);
}

void UploadJob::uploadInfoMessage(KJob*, const QString& plain)
{
    m_progressDialog->setLabelText(plain);
}

void UploadJob::setOutputModel(QStandardItemModel* model)
{
    m_outputModel = model;
}
QStandardItemModel* UploadJob::outputModel()
{
    return m_outputModel;
}
QStandardItem* UploadJob::appendLog(const QString& message)
{
    if (m_outputModel) {
        QStandardItem* item = new QStandardItem(message);
        m_outputModel->appendRow(item);
        return item;
    } else {
        return nullptr;
    }
}
void UploadJob::setQuickUpload(bool v)
{
    m_quickUpload = v;
}

bool UploadJob::isQuickUpload()
{
    return m_quickUpload;
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
