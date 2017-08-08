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

#include <interfaces/iplugin.h>

class QSignalMapper;
class QStandardItemModel;
class KActionMenu;
class QAction;
namespace KDevelop {
  class ProjectBaseItem;
  class IProject;
  class IDocument;
}
class UploadProfileModel;
class FilesTreeViewFactory;
class AllProfilesModel;

class UploadPlugin : public KDevelop::IPlugin
{
Q_OBJECT
public:
    UploadPlugin(QObject *parent, const QVariantList & = QVariantList() );
    ~UploadPlugin() override;

    /**
    * Returns the Upload-Action for the Contextmenu.
    */
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    /**
    * Returns (and creates) the outputModel used for UploadPlugin.
    * Creates the output-view (only the first time called)
    */
    QStandardItemModel* outputModel();
    
    int perProjectConfigPages() const override;
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

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

    void quickUploadCurrentFile();

    /**
    * Called when project was opened, adds a upload-action to the project-menu.
    */
    void projectOpened(KDevelop::IProject*);

    /**
    * Called when project was closed, removes the upload-action from the project-menu.
    */
    void projectClosed(KDevelop::IProject*);

    /**
    * Checks if there are any upload profiles, hides the ProfilesFileTree if there are none.
    */
    void profilesRowChanged();

    void documentActivated(KDevelop::IDocument*);
    void documentClosed(KDevelop::IDocument*);

private:
    void setupActions();

    QList<KDevelop::ProjectBaseItem*> m_ctxUrlList; ///< selected files when the contextmenu was requested

    KActionMenu* m_projectUploadActionMenu; ///< upload ActionMenu, displayed in the Project-Menu
    QAction* m_quickUploadCurrentFile;
    QMap<KDevelop::IProject*, QAction*> m_projectUploadActions; ///< upload actions for every open project
    QMap<KDevelop::IProject*, UploadProfileModel*> m_projectProfileModels; ///< UploadProfileModels for every open project
    QSignalMapper* m_signalMapper; ///< signal mapper for upload actions, to get the correct project
    QStandardItemModel* m_outputModel; ///< model for log-output
    FilesTreeViewFactory* m_filesTreeViewFactory; ///< factory for ProjectFilesTree
    AllProfilesModel* m_allProfilesModel; ///< model for all profiles
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
