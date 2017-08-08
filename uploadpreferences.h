/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef UPLOADPREFERENCES_H
#define UPLOADPREFERENCES_H

#include <ksharedconfig.h>
#include <QVariant>

#include <project/projectconfigpage.h>

#include "uploadconfig.h"

namespace Ui {
    class UploadPreferences;
}
class UploadProfileDlg;
class UploadProfileModel;
class UploadProfileItem;

/**
 * KCModule for upload profiles configuration
 */
class UploadPreferences : public ProjectConfigPage<UploadSettings>
{
    Q_OBJECT
public:

    UploadPreferences(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent);
    ~UploadPreferences() override;

    void reset() override;
    void apply() override;
    void defaults() override;
    
    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

private Q_SLOTS:
    void addProfile();
    void removeProfile();
    void modifyProfile();

private:
    Ui::UploadPreferences* m_ui;
    UploadProfileDlg* m_dlg;
    UploadProfileModel* m_model;
};


#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
