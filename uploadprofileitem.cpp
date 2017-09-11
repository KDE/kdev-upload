/***************************************************************************
*   Copyright 2007 Niko Sams <niko.sams@gmail.com>                        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "uploadprofileitem.h"

#include <QUrl>

#include <kconfiggroup.h>

#include <interfaces/iproject.h>

#include "uploadprofilemodel.h"

UploadProfileItem::UploadProfileItem()
{
    setEditable(false);
}

void UploadProfileItem::setUrl(const QUrl& url)
{
    setData(url, UrlRole);
}
void UploadProfileItem::setLocalUrl(const QUrl& url)
{
    setData(url, LocalUrlRole);
}

void UploadProfileItem::setDefault(bool isDefault)
{
    setData(isDefault, IsDefaultRole);
    if (isDefault) {
        setIcon(QIcon::fromTheme("rating"));
        UploadProfileModel* m;
        if (model() && (m = dynamic_cast<UploadProfileModel*>(model()))) {
            for (int i = 0; i < m->rowCount(); i++) {
                UploadProfileItem* item = m->uploadItem(i);
                if (item && item != this) {
                    item->setDefault(false);
                }
            }
        }
    } else {
        setIcon(QIcon());
    }
}

void UploadProfileItem::setProfileNr(const QString& nr)
{
    setData(nr, ProfileNrRole);
}

QUrl UploadProfileItem::url() const
{
    return data(UrlRole).value<QUrl>();
}
QUrl UploadProfileItem::localUrl() const
{
    return data(LocalUrlRole).value<QUrl>();
}

bool UploadProfileItem::isDefault() const
{
    return data(IsDefaultRole).toBool();
}

QString UploadProfileItem::profileNr() const
{
    return data(ProfileNrRole).toString();
}

KConfigGroup UploadProfileItem::profileConfigGroup() const
{
    UploadProfileModel* m = nullptr;
    if (!profileNr().isEmpty() && model() && (m = dynamic_cast<UploadProfileModel*>(model()))) {
        return m->project()->projectConfiguration()->group("Upload").group("Profile"+profileNr());
    }
    return KConfigGroup();
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
