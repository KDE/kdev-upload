/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KDEVUPLOADPLUGIN_H
#define KDEVUPLOADPLUGIN_H

#include <QList>
#include <QtCore/QVariant>

#include <kurl.h>

#include "iplugin.h"

namespace KDevelop {
  class ProjectBaseItem;
  class IProject;
}
class UploadProfileModel;

class UploadPlugin : public KDevelop::IPlugin
{
Q_OBJECT
public:
    UploadPlugin(QObject *parent, const QVariantList & = QVariantList() );
    ~UploadPlugin();
    void unload();

    /**
    * Returns the Upload-Action for the Contextmenu.
    */
    QPair<QString,QList<QAction*> > requestContextMenuActions( KDevelop::Context* );

public Q_SLOTS:
    /**
    * Opens the UploadDialog with the previously selected file as root.
    */
    void upload();

    /**
    * Uploads the previously selected file without any futher actions needed by the user
    */
    void quickUpload();


private:
    QList<KDevelop::ProjectBaseItem*> m_ctxUrlList;
    QMap<KDevelop::IProject*, UploadProfileModel*> m_profileModels;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
