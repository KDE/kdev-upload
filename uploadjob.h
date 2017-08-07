/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef UPLOADJOB_H
#define UPLOADJOB_H

#include <QDialog>
#include <QModelIndex>

class QProgressDialog;
class KJob;
namespace KIO {
    class Job;
    class CopyJob;
    class JobUiDelegate;
    class NetAccess;
}
namespace KDevelop {
    class IProject;
    class ProjectBaseItem;
}
class QStandardItemModel;
class QStandardItem;
class UploadProjectModel;
class UploadPlugin;

/**
 * Class that does the Uploading.
 * Used for Quick-Upload and in UploadDialog
 */
class UploadJob : public QObject
{
    Q_OBJECT

public:
    UploadJob(KDevelop::IProject* project, UploadProjectModel* model, QWidget *parent = nullptr);
    ~UploadJob() override;

    /**
     * Sets if the files should only be marked as uploaded
     */
    void setOnlyMarkUploaded(bool v) {
        m_onlyMarkUploaded = v;
    }
    bool onlyMarkUploaded() {
        return m_onlyMarkUploaded;
    }

    /**
     * Sets if the upload is a quick upload.
     * If true, unmodified files will be logged too
     */
    void setQuickUpload(bool q);
    bool isQuickUpload();


    /**
     * Sets the output model that should be used to output the log messages
     */
    void setOutputModel(QStandardItemModel* model);
    QStandardItemModel* outputModel();

public Q_SLOTS:
    /**
     * Starts the upload
     */
    void start();

private Q_SLOTS:
    /**
     * Upload the next (or first) item
     */
    void uploadNext();

    /**
     * Called when current job is finished
     */
    void uploadResult(KJob*);

    /**
     * Updates the progress bar
     */
    void processedSize(KJob*, qulonglong);

    /**
     * Updates the progress text
     */
    void uploadInfoMessage(KJob*, const QString& plain);

    /**
     * Cancel button in the ProgressDialog clicked
     */
    void cancelClicked();

Q_SIGNALS:
    /**
     * Signal is emitted when the upload successfully finished
     */
    void uploadFinished();

private:
    /**
     * Appends a message to the current outputModel.
     * @return the QStandardItem* to modify it further (to eg. change color)
     */
    QStandardItem* appendLog(const QString& message);
    
    QModelIndex m_uploadIndex; ///< current index when the upload is running

    KDevelop::IProject* m_project; ///< the project of this job
    UploadProjectModel* m_uploadProjectModel;

    QProgressDialog* m_progressDialog; ///< progress-dialog when the upload is running
    int m_progressBytesDone; ///< uploaded bytes, incremented when a file is fully uploaded. used for progress.

    bool m_onlyMarkUploaded; ///< if files should be only marked as uploaded
    bool m_quickUpload; ///< if it is a quick upload

    QStandardItemModel* m_outputModel;
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
