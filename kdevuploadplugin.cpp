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

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kparts/mainwindow.h>

#include <icore.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <iuicontroller.h>
#include <context.h>
#include <projectmodel.h>

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
: KDevelop::IPlugin(UploadFactory::componentData(), parent)
{
    FilesTreeViewFactory *factory = new FilesTreeViewFactory(this);
    core()->uiController()->addToolView( i18n("Upload Profiles"), factory );
}

UploadPlugin::~UploadPlugin()
{
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

    UploadDialog dialog(project, core()->uiController()->activeMainWindow());
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
    job->start();
}

#include "kdevuploadplugin.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
