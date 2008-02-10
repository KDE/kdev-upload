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

class KProgressDialog;
class KJob;
namespace KDevelop {
    class IProject;
    class ProjectBaseItem;
}
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
    UploadJob(KDevelop::IProject* project, UploadProjectModel* model, QWidget *parent = 0);
    virtual ~UploadJob();

    /**
     * Sets if the files should only be marked as uploaded
     */
    void setOnlyMarkUploaded(bool v) {
        m_onlyMarkUploaded = v;
    }
    bool onlyMarkUploaded() {
        return m_onlyMarkUploaded;
    }

public Q_SLOTS:
    /**
     * Starts the upload
     */
    void start();

private Q_SLOTS:
    void uploadNext();
    void uploadResult(KJob*);
    void processedSize(KJob*, qulonglong);
    void uploadInfoMessage(KJob*, const QString& plain);

private:
    void handleError(KJob* job);
    
    QModelIndex m_uploadIndex;

    KDevelop::IProject* m_project;
    UploadProjectModel* m_uploadProjectModel;

    KProgressDialog* m_progressDialog;
    int m_progressBytesDone;

    bool m_onlyMarkUploaded;
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
