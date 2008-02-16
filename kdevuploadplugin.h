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

class QSignalMapper;
class QStandardItemModel;
class KActionMenu;
class KAction;
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

    /**
    * Returns (and creates) the outputModel used for UploadPlugin.
    * Creates the output-view (only the first time called)
    */
    QStandardItemModel* outputModel();

private Q_SLOTS:
    /**
    * Opens the UploadDialog with the previously selected file as root.
    * Executed from context-menu
    */
    void upload();

    /**
    * Uploads the previously selected file without any futher actions needed by the user.
    * Executed from context-menu
    */
    void quickUpload();

    /**
    * Opens the UploadDialog for the project.
    * Executed by the upload-action in the Project-menu
    */
    void projectUpload(QObject* project);

    /**
    * Called when project was opened, adds a upload-action to the project-menu.
    */
    void projectOpened(KDevelop::IProject*);

    /**
    * Called when project was closed, removes the upload-action from the project-menu.
    */
    void projectClosed(KDevelop::IProject*);

private:
    void setupActions();

    QList<KDevelop::ProjectBaseItem*> m_ctxUrlList; ///< selected files when the contextmenu was requested

    KActionMenu* m_projectUploadActionMenu; ///< upload ActionMenu, displayed in the Project-Menu
    QMap<KDevelop::IProject*, KAction*> m_projectUploadActions; ///< upload actions for every open project
    QSignalMapper* m_signalMapper; ///< signal mapper for upload actions, to get the correct project
    QStandardItemModel* m_outputModel; ///< model for log-output
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
