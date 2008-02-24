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

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kparts/mainwindow.h>
#include <kactioncollection.h>
#include <kactionmenu.h>

#include <icore.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <iuicontroller.h>
#include <context.h>
#include <projectmodel.h>
#include <core.h>
#include <ioutputview.h>
#include <plugincontroller.h>

#include "uploaddialog.h"
#include "profilesfiletree.h"
#include "uploadjob.h"
#include "uploadprojectmodel.h"
#include "uploadprofilemodel.h"
#include "uploadprofileitem.h"

K_PLUGIN_FACTORY(UploadFactory, registerPlugin<UploadPlugin>(); )
K_EXPORT_PLUGIN(UploadFactory("kdevupload"))

class FilesTreeViewFactory: public KDevelop::IToolViewFactory{
  public:
    FilesTreeViewFactory(UploadPlugin *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0)
    {
        return new ProfilesFileTree(m_plugin, parent);
    }

    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::RightDockWidgetArea;
    }

  private:
      UploadPlugin *m_plugin;
};

UploadPlugin::UploadPlugin(QObject *parent, const QVariantList &)
: KDevelop::IPlugin(UploadFactory::componentData(), parent),  m_outputModel(0)
{
    connect(core()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)),
                   this, SLOT(projectOpened(KDevelop::IProject*)));
    connect(core()->projectController(), SIGNAL(projectClosed(KDevelop::IProject*)),
                   this, SLOT(projectClosed(KDevelop::IProject*)));

    setXMLFile("kdevupload.rc");

    setupActions();

    FilesTreeViewFactory *factory = new FilesTreeViewFactory(this);
    core()->uiController()->addToolView( i18n("Upload Profiles"), factory );
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
    m_projectUploadActionMenu->setIcon(KIcon("go-up"));
    m_projectUploadActionMenu->setToolTip(i18n("Upload project"));
    actionCollection()->addAction("project_upload", m_projectUploadActionMenu);
}

void UploadPlugin::projectOpened(KDevelop::IProject* project)
{
    KAction* action = new KAction(project->name(), m_projectUploadActionMenu);
    connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(action, project);
    m_projectUploadActions.insert(project, action);
    m_projectUploadActionMenu->addAction(action);
}
void UploadPlugin::projectClosed(KDevelop::IProject* project)
{
    KAction* action = m_projectUploadActions.value(project);
    if (action) {
        m_projectUploadActionMenu->removeAction(action);
        delete action;
    }
}

void UploadPlugin::projectUpload(QObject* p)
{
    KDevelop::IProject* project = qobject_cast<KDevelop::IProject*>(p);
    if (project) {
        UploadDialog dialog(project, this, core()->uiController()->activeMainWindow());
        dialog.exec();
    }
}

void UploadPlugin::unload()
{
}

QPair<QString,QList<QAction*> > UploadPlugin::requestContextMenuActions(KDevelop::Context* context)
{
    if (context->type() == KDevelop::Context::ProjectItemContext) {
        KDevelop::ProjectItemContext *itemCtx = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        if (itemCtx) {
            m_ctxUrlList.clear();
            Q_FOREACH (KDevelop::ProjectBaseItem* _item, itemCtx->items()) {
                if (_item->folder() || _item->file()) {
                    m_ctxUrlList << _item;
                }
            }
            if (!m_ctxUrlList.isEmpty()) {
                QList<QAction*> actions;
                QAction *action;
                action = new QAction(i18n("Upload..."), this);
                connect(action, SIGNAL(triggered()), this, SLOT(upload()));
                actions << action;

                action = new QAction(i18n("Quick Upload"), this);
                connect(action, SIGNAL(triggered()), this, SLOT(quickUpload()));
                actions << action;

                return qMakePair(QString("Upload"), actions);
            }
        }
    }
    return KDevelop::IPlugin::requestContextMenuActions(context);
}

void UploadPlugin::upload()
{
    if (m_ctxUrlList.isEmpty()) return;
    KDevelop::IProject* project = m_ctxUrlList.at(0)->project();

    UploadDialog dialog(project, this, core()->uiController()->activeMainWindow());
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

    UploadProfileModel* profileModel = new UploadProfileModel();
    profileModel->setConfig(project->projectConfiguration());
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
        int id = view->registerView(i18n("Upload"), KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);

        m_outputModel = new QStandardItemModel(this);

        view->setModel(id, m_outputModel);
        view->setDelegate(id, new QItemDelegate(m_outputModel));

        return m_outputModel;
    }
    return 0;
}

#include "kdevuploadplugin.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
