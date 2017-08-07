/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "kdevuploadplugin.h"

#include <QAction>
#include <QVBoxLayout>
#include <QSignalMapper>
#include <QStandardItemModel>
#include <QItemDelegate>
#include "kdevuploaddebug.h"

#include <KPluginFactory>
#include <KLocalizedString>
#include <kparts/mainwindow.h>
#include <kactioncollection.h>
#include <kactionmenu.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <project/projectmodel.h>
#include <outputview/ioutputview.h>
#include <serialization/indexedstring.h>

#include "uploaddialog.h"
#include "profilesfiletree.h"
#include "uploadjob.h"
#include "uploadprojectmodel.h"
#include "uploadprofilemodel.h"
#include "uploadprofileitem.h"
#include "allprofilesmodel.h"
#include <interfaces/idocumentcontroller.h>

#include "version.h"

K_PLUGIN_FACTORY_WITH_JSON(UploadFactory, "kdevupload.json", registerPlugin<UploadPlugin>(); )

Q_LOGGING_CATEGORY(KDEVUPLOAD, "kdev.upload");

class FilesTreeViewFactory: public KDevelop::IToolViewFactory{
  public:
    FilesTreeViewFactory(UploadPlugin* plugin, AllProfilesModel* model)
                : m_plugin(plugin), m_allProfilesModel(model) {}

    virtual QWidget* create(QWidget *parent = 0)
    {
        ProfilesFileTree* w = new ProfilesFileTree(m_plugin, parent);
        w->setModel(m_allProfilesModel);
        return w;
    }

    virtual QString id() const
    {
        return "org.quanta.UploadFactory";
    }

    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::RightDockWidgetArea;
    }

    private:
        UploadPlugin* m_plugin;
        AllProfilesModel* m_allProfilesModel;
};

UploadPlugin::UploadPlugin(QObject *parent, const QVariantList &)
: KDevelop::IPlugin(QStringLiteral("kdevupload"), parent),  m_outputModel(0), m_filesTreeViewFactory(0)
{
    connect(core()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)),
                   this, SLOT(projectOpened(KDevelop::IProject*)));
    connect(core()->projectController(), SIGNAL(projectClosed(KDevelop::IProject*)),
                   this, SLOT(projectClosed(KDevelop::IProject*)));
    connect(core()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)),
                SLOT(documentActivated(KDevelop::IDocument*)));
    connect(core()->documentController(), SIGNAL(documentClosed(KDevelop::IDocument*)),
                SLOT(documentClosed(KDevelop::IDocument*)));

    setXMLFile( QStringLiteral( "kdevupload.rc" ) );

    m_allProfilesModel = new AllProfilesModel(this);
    connect(m_allProfilesModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
                    this, SLOT(profilesRowChanged()));
    connect(m_allProfilesModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
                    this, SLOT(profilesRowChanged()));
    connect(m_allProfilesModel, SIGNAL(modelReset()),
                    this, SLOT(profilesRowChanged()));

    setupActions();
}

UploadPlugin::~UploadPlugin()
{
}

void UploadPlugin::setupActions()
{
    m_signalMapper = new QSignalMapper(this);
    connect(m_signalMapper, SIGNAL(mapped(QObject*)),
            this, SLOT(projectUpload(QObject*)));
    m_projectUploadActionMenu = new KActionMenu(i18n("&Upload Project"), this);
    m_projectUploadActionMenu->setIcon(QIcon::fromTheme("go-up"));
    m_projectUploadActionMenu->setToolTip(i18n("Upload project"));
    m_projectUploadActionMenu->setVisible(false); //make it visible when there are upload profiles
    actionCollection()->addAction("project_upload", m_projectUploadActionMenu);

    m_quickUploadCurrentFile = actionCollection()->addAction("quick_upload_current_file");
    m_quickUploadCurrentFile->setText( i18n("&Quick Upload Current File") );
    m_quickUploadCurrentFile->setIcon(QIcon::fromTheme("go-up"));
    m_projectUploadActionMenu->setEnabled(false);
    connect(m_quickUploadCurrentFile, SIGNAL(triggered(bool)), SLOT(quickUploadCurrentFile()));
}

void UploadPlugin::projectOpened(KDevelop::IProject* project)
{
    UploadProfileModel* model = new UploadProfileModel();
    model->setProject(project);
    m_projectProfileModels.insert(project, model);
    m_allProfilesModel->addModel(model);

    documentActivated(core()->documentController()->activeDocument());
}

void UploadPlugin::projectClosed(KDevelop::IProject* project)
{
    QAction* action = m_projectUploadActions.value(project);
    if (action) {
        m_projectUploadActions.remove(project);
        m_projectUploadActionMenu->removeAction(action);
        delete action;
    }
    UploadProfileModel* model = m_projectProfileModels.value(project);
    if (model) {
        m_projectProfileModels.remove(project);
        m_allProfilesModel->removeModel(model);
        delete model;
    }
}


void UploadPlugin::documentActivated(KDevelop::IDocument* doc)
{
    if (!doc) {
        m_quickUploadCurrentFile->setEnabled(false);
        return;
    }
    KDevelop::IProject* project = core()->projectController()->findProjectForUrl(doc->url());
    if (!project) {
        m_quickUploadCurrentFile->setEnabled(false);
        return;
    }
    QList<KDevelop::ProjectFileItem*> files = project->filesForPath(KDevelop::IndexedString(doc->url()));
    if (files.isEmpty()) {
        m_quickUploadCurrentFile->setEnabled(false);
        return;
    }
    UploadProfileModel* profileModel = m_projectProfileModels.value(project);
    if (!profileModel || !profileModel->rowCount()) {
        m_quickUploadCurrentFile->setEnabled(false);
        return;
    }

    m_quickUploadCurrentFile->setEnabled(true);
}


void UploadPlugin::documentClosed(KDevelop::IDocument* )
{
    m_quickUploadCurrentFile->setEnabled(false);
}


void UploadPlugin::projectUpload(QObject* p)
{
    KDevelop::IProject* project = qobject_cast<KDevelop::IProject*>(p);
    if (project) {
        UploadDialog dialog(project, this, core()->uiController()->activeMainWindow());
        UploadProfileModel* model = m_projectProfileModels.value(project);
        dialog.setProfileModel(model);
        dialog.exec();
    }
}

void UploadPlugin::unload()
{
}

KDevelop::ContextMenuExtension UploadPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    if (context->type() == KDevelop::Context::ProjectItemContext) {
        KDevelop::ContextMenuExtension cmExtension;
        KDevelop::ProjectItemContext *itemCtx = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        if (itemCtx) {
            m_ctxUrlList.clear();
            Q_FOREACH (KDevelop::ProjectBaseItem* _item, itemCtx->items()) {
                if (_item->folder() || _item->file()) {
                    m_ctxUrlList << _item;
                }
            }
            if (!m_ctxUrlList.isEmpty()) {
                KDevelop::IProject* project = m_ctxUrlList.at(0)->project();
                UploadProfileModel* model = m_projectProfileModels.value(project);
                if (model && model->rowCount()) {
                    QAction *action;
                    action = new QAction(i18n("Upload..."), parent);
                    action->setIcon(QIcon::fromTheme("go-up"));
                    connect(action, SIGNAL(triggered()), this, SLOT(upload()));
                    cmExtension.addAction(KDevelop::ContextMenuExtension::FileGroup, action);
    
                    action = new QAction(i18n("Quick Upload"), parent);
                    action->setIcon(QIcon::fromTheme("go-up"));
                    connect(action, SIGNAL(triggered()), this, SLOT(quickUpload()));
                    cmExtension.addAction(KDevelop::ContextMenuExtension::FileGroup, action);
                    
                    return cmExtension;
                }
            }
        }
    }
    return KDevelop::IPlugin::contextMenuExtension(context, parent);
}

void UploadPlugin::upload()
{
    if (m_ctxUrlList.isEmpty()) return;
    KDevelop::IProject* project = m_ctxUrlList.at(0)->project();

    UploadDialog dialog(project, this, core()->uiController()->activeMainWindow());
    UploadProfileModel* model = m_projectProfileModels.value(project);
    dialog.setProfileModel(model);
    dialog.setRootItem(m_ctxUrlList.at(0));
    dialog.exec();
}
void UploadPlugin::quickUpload()
{
    if (m_ctxUrlList.isEmpty()) return;
    KDevelop::IProject* project = m_ctxUrlList.at(0)->project();

    UploadProjectModel* model = new UploadProjectModel(project);
    model->setSourceModel(project->projectItem()->model());
    model->setRootItem(m_ctxUrlList.at(0));

    UploadProfileModel* profileModel = m_projectProfileModels.value(project);
    for (int i = 0; i < profileModel->rowCount(); i++) {
        UploadProfileItem* item = profileModel->uploadItem(i);
        if (item->isDefault()) {
            KConfigGroup c = item->profileConfigGroup();
            if (c.isValid()) {
                model->setProfileConfigGroup(c);
            }
            break;
        }
    }

    UploadJob* job = new UploadJob(project, model, core()->uiController()->activeMainWindow());
    job->setQuickUpload(true);
    job->setOutputModel(outputModel());
    job->start();
}


void UploadPlugin::quickUploadCurrentFile()
{
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    if (!doc) return;
    KDevelop::IProject* project = KDevelop::ICore::self()->projectController()->findProjectForUrl(doc->url());
    if (!project) return;
    QList<KDevelop::ProjectFileItem*> files = project->filesForPath(KDevelop::IndexedString(doc->url()));
    if (files.isEmpty()) return;

    UploadProjectModel* model = new UploadProjectModel(project);
    model->setSourceModel(project->projectItem()->model());
    model->setRootItem(files.first());

    UploadProfileModel* profileModel = m_projectProfileModels.value(project);
    for (int i = 0; i < profileModel->rowCount(); i++) {
        UploadProfileItem* item = profileModel->uploadItem(i);
        if (item->isDefault()) {
            KConfigGroup c = item->profileConfigGroup();
            if (c.isValid()) {
                model->setProfileConfigGroup(c);
            }
            break;
        }
    }

    UploadJob* job = new UploadJob(project, model, core()->uiController()->activeMainWindow());
    job->setQuickUpload(true);
    job->setOutputModel(outputModel());
    job->start();

}


QStandardItemModel* UploadPlugin::outputModel()
{
    if (m_outputModel) return m_outputModel;
    IPlugin* plugin = core()->pluginController()->pluginForExtension( "org.kdevelop.IOutputView" );
    Q_ASSERT(plugin);
    if (plugin) {
        KDevelop::IOutputView* view = plugin->extension<KDevelop::IOutputView>();
        int tvid = view->registerToolView(i18n("Upload"));
        int id = view->registerOutputInToolView(tvid, i18n("Output"), KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
        m_outputModel = new QStandardItemModel(this);

        view->setModel(id, m_outputModel);
        view->setDelegate(id, new QItemDelegate(m_outputModel));

        return m_outputModel;
    }
    return 0;
}

void UploadPlugin::profilesRowChanged()
{
    if (m_allProfilesModel->rowCount()) {
        if (!m_filesTreeViewFactory) {
            m_filesTreeViewFactory = new FilesTreeViewFactory(this, m_allProfilesModel);
            core()->uiController()->addToolView(i18n("Upload Profiles"), m_filesTreeViewFactory);
        }
    } else {
        if (m_filesTreeViewFactory) {
            core()->uiController()->removeToolView(m_filesTreeViewFactory);
            m_filesTreeViewFactory = 0;
        }
    }
    Q_FOREACH(UploadProfileModel* model, m_projectProfileModels) {
        KDevelop::IProject* project = model->project();
        
        if (model->rowCount()) {
            if (!m_projectUploadActions.contains(project)) {
                QAction* action = new QAction(project->name(), m_projectUploadActionMenu);
                connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
                m_signalMapper->setMapping(action, project);
                m_projectUploadActions.insert(project, action);
                m_projectUploadActionMenu->addAction(action);
                m_projectUploadActionMenu->setVisible(true);
            }
        } else {
            if (m_projectUploadActions.contains(project)) {
                QAction* action = m_projectUploadActions.value(project);
                m_projectUploadActions.remove(project);
                m_projectUploadActionMenu->removeAction(action);
                m_signalMapper->removeMappings(action);
                delete action;
            }
        }
    }
    
    if (m_projectUploadActions.isEmpty()) {
        m_projectUploadActionMenu->setVisible(false);
    }
}

#include "kdevuploadplugin.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
