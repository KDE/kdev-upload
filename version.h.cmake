/*****************************************************************************
 * This file is part of the KDE project
 * Copyright 2013 Milian Wolff <mail@milianw.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *******************************************************************************/
#ifndef KDEVUPLOAD_VERSION_H
#define KDEVUPLOAD_VERSION_H

#include <kdeversion.h>

#define KDEVUPLOAD_MAJOR_VERSION @KDEVUPLOAD_VERSION_MAJOR@
#define KDEVUPLOAD_MINOR_VERSION @KDEVUPLOAD_VERSION_MINOR@
#define KDEVUPLOAD_PATCH_VERSION @KDEVUPLOAD_VERSION_PATCH@

#define KDEVUPLOAD_VERSION_STR "@KDEVUPLOAD_VERSION_MAJOR@.@KDEVUPLOAD_VERSION_MINOR@.@KDEVUPLOAD_VERSION_PATCH@"

#define KDEVUPLOAD_VERSION KDE_MAKE_VERSION(@KDEVUPLOAD_VERSION_MAJOR@, @KDEVUPLOAD_VERSION_MINOR@, @KDEVUPLOAD_VERSION_PATCH@)

#endif
