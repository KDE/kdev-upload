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

#include <kprogressdialog.h>
#include <kconfiggroup.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/jobuidelegate.h>
#include <kio/netaccess.h>
#include <klocale.h>

#include <iproject.h>
#include <iprojectcontroller.h>
#include <projectmodel.h>

#include "uploadprojectmodel.h"

UploadJob::UploadJob(KDevelop::IProject* project, UploadProjectModel* model, QWidget *parent)
    : QObject(parent), m_project(project), m_uploadProjectModel(model),
      m_onlyMarkUploaded(false), m_outputModel(0)
{
    m_progressDialog = new KProgressDialog(parent,
                                            i18n("Uploading files"),
                                            i18n("Preparing..."));
    m_progressDialog->setWindowModality(Qt::WindowModal);
    m_progressDialog->setAutoClose(false);
}

UploadJob::~UploadJob()
{
    delete m_progressDialog;
}

void UploadJob::start()
{
    m_progressBytesDone = 0;
    m_progressDialog->setLabelText(i18n("Calculating size..."));
    m_progressDialog->progressBar()->setValue(0);
    m_progressDialog->show();

    int sumSize = 0;
    QModelIndex i;
    while((i = m_uploadProjectModel->nextRecursionIndex(i)).isValid()) {
        KDevelop::ProjectBaseItem* item = m_uploadProjectModel->item(i);
        Qt::CheckState checked = static_cast<Qt::CheckState>(m_uploadProjectModel
                                ->data(i, Qt::CheckStateRole).toInt());
        if (item->file() && checked != Qt::Unchecked) {
            KIO::UDSEntry entry;
            if (KIO::NetAccess::stat(item->file()->url(), entry, m_progressDialog)) {
                sumSize += entry.numberValue(KIO::UDSEntry::UDS_SIZE);
            }
        }
    }
    m_progressDialog->progressBar()->setMaximum(sumSize);

    m_uploadIndex = QModelIndex();
    uploadNext();
}

void UploadJob::uploadNext()
{
    if (m_progressDialog->wasCancelled()) return;

    m_uploadIndex = m_uploadProjectModel->nextRecursionIndex(m_uploadIndex);

    if (!m_uploadIndex.isValid()) {
        //last index reached - completed
        appendLog(i18n("Upload completed"));
        m_progressDialog->close();
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
    KUrl url;
    if (item->folder()) url = item->folder()->url();
    else if (item->file()) url = item->file()->url();

    if (!(item->file() || item->folder()) || checked == Qt::Unchecked) {
        uploadNext();
        return;
    }

    KUrl dest = m_uploadProjectModel->currentProfileUrl();
    dest.addPath(KUrl::relativeUrl(m_project->folder(), url));

    KIO::Job* job = 0;

    if (m_onlyMarkUploaded) {
        appendLog(i18n("Marked as uploaded for %1: %2",
                            m_uploadProjectModel->currentProfileName(),
                            KUrl::relativeUrl(m_project->folder(), url)));
        m_uploadProjectModel->profileConfigGroup()
                .writeEntry(KUrl::relativeUrl(m_project->folder(), url),
                            QDateTime::currentDateTime());
        uploadNext();
        return;
    } else if (item->file()) {
        appendLog(i18n("Uploading to %1: %2",
                            m_uploadProjectModel->currentProfileName(),
                            KUrl::relativeUrl(m_project->folder(), url)));
        kDebug() << "file_copy" << url << dest;
        job = KIO::file_copy(url, dest, -1, KIO::Overwrite | KIO::HideProgressInfo);
        m_progressDialog->setLabelText(i18n("Uploading %1...", KUrl::relativeUrl(m_project->folder(), url)));
    } else if (item->folder()) {
        if (KIO::NetAccess::exists(dest, KIO::NetAccess::DestinationSide, m_progressDialog)) {
            appendLog(i18n("Directory in %1 allready exists: %2", 
                                m_uploadProjectModel->currentProfileName(),
                                KUrl::relativeUrl(m_project->folder(), url)));
            m_uploadProjectModel->profileConfigGroup()
                    .writeEntry(KUrl::relativeUrl(m_project->folder(), url),
                                QDateTime::currentDateTime());
            uploadNext();
            return;
        } else {
            appendLog(i18n("Creating directory in %1: %2", 
                                m_uploadProjectModel->currentProfileName(),
                                KUrl::relativeUrl(m_project->folder(), url)));
            kDebug() << "mkdir" << dest;
            job = KIO::mkdir(dest);
        }
    } else {
        uploadNext();
        return;
    }

    job->ui()->setWindow(m_progressDialog);
    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(uploadResult(KJob*)));
    connect(job, SIGNAL(processedSize(KJob*, qulonglong)),
            this, SLOT(processedSize(KJob*, qulonglong)));
    connect(job, SIGNAL(infoMessage(KJob*, QString)),
            this, SLOT(uploadInfoMessage(KJob*, QString)));

    connect(m_progressDialog, SIGNAL(cancelClicked()),
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
            m_progressDialog->close();
            delete this;
            return;
        }
        appendLog(i18n("Upload error: %1", job->errorString()));
        m_progressDialog->close();
        qobject_cast<KIO::Job*>(job)->ui()->showErrorMessage();
        delete this;
        return;
    }

    KDevelop::ProjectBaseItem* item = m_uploadProjectModel->item(m_uploadIndex);
    KUrl url;
    if (item->file()) {
        url = item->file()->url();
    } else if (item->folder()) {
        url = item->folder()->url();
    }
    m_uploadProjectModel->profileConfigGroup()
        .writeEntry(KUrl::relativeUrl(m_project->folder(), url), QDateTime::currentDateTime());
    m_uploadProjectModel->profileConfigGroup().sync();

    KIO::UDSEntry entry;
    if (KIO::NetAccess::stat(url, entry, m_progressDialog)) {
        m_progressBytesDone += entry.numberValue(KIO::UDSEntry::UDS_SIZE);
    }
    m_progressDialog->progressBar()->setValue(m_progressBytesDone);

    uploadNext();
}

void UploadJob::processedSize(KJob*, qulonglong size)
{
    m_progressDialog->progressBar()->setValue(m_progressBytesDone + size);
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
        return 0;
    }
}

#include "uploadjob.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
