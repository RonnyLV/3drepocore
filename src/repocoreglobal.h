/**
 *  Copyright (C) 2014 3D Repo Ltd
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// http://qt-project.org/doc/qt-5/sharedlibrary.html

#ifndef REPO_CORE_GLOBAL_H
#define REPO_CORE_GLOBAL_H

#pragma warning( disable : 4996 )
#pragma warning( disable : 4267 )
#pragma warning( disable : 4251 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4100 )
#pragma warning( disable : 4005 )

//------------------------------------------------------------------------------
#if defined(_WIN32) || defined(_WIN64)
#   define Q_DECL_EXPORT __declspec(dllexport)
#   define Q_DECL_IMPORT __declspec(dllimport)
#else
#   define Q_DECL_EXPORT
#   define Q_DECL_IMPORT
#endif

//------------------------------------------------------------------------------
#if defined(REPO_CORE_LIBRARY)
#   define REPO_CORE_EXPORT Q_DECL_EXPORT
#else
#   define REPO_CORE_EXPORT Q_DECL_IMPORT
#endif

//------------------------------------------------------------------------------
// Constants
#define REPO_ADMIN                  "admin"             //!< Admin database
#define REPO_SYSTEM_USERS           "system.users"      //!< Users collection
#define REPO_SYSTEM_ROLES           "system.roles"      //!< Roles collection

#define REPO_LABEL_ID               "_id"
#define REPO_LABEL_AVATAR           "avatar"
#define REPO_LABEL_DATA             "data"
#define REPO_LABEL_HEIGHT           "height"
#define REPO_LABEL_MEDIA_TYPE       "media_type"
#define REPO_LABEL_ROLE             "role"
#define REPO_LABEL_ROLES            "roles"
#define REPO_LABEL_USER             "user"
#define REPO_LABEL_WIDTH            "width"
#define REPO_LABEL_TYPE             "type"

#define REPO_LABEL_DESCRIPTION      "desc"
#define REPO_LABEL_OWNER            "owner"
#define REPO_LABEL_GROUP            "group"
#define REPO_LABEL_PERMISSIONS      "permissions"
#define REPO_LABEL_USERS            "users"

#define REPO_COMMAND_UPDATE         "update"
#define REPO_COMMAND_UPDATES        "updates"
#define REPO_COMMAND_DELETE         "delete"
#define REPO_COMMAND_DELETES        "deletes"
#define REPO_COMMAND_UPSERT         "upsert"
#define REPO_COMMAND_LIMIT          "limit"
#define REPO_COMMAND_Q              "q"
#define REPO_COMMAND_U              "u"

#define REPO_COLLECTION_HISTORY     "history"
#define REPO_COLLECTION_SCENE       "scene"
#define REPO_COLLECTION_SETTINGS    "settings"

//------------------------------------------------------------------------------
// Media Types a.k.a. as Multipurpose Internet Mail Extensions (MIME) Types
// http://www.iana.org/assignments/media-types/media-types.xhtml#image
// http://doc.qt.io/qt-5/qimagereader.html

#define REPO_MEDIA_TYPE_BMP     "image/bmp"                 //!< Windows Bitmap
#define REPO_MEDIA_TYPE_GIF     "image/gif"                 //!< Graphic Interchange Format
#define REPO_MEDIA_TYPE_JPG     "image/jpeg"                //!< Joint Photographic Experts Group
#define REPO_MEDIA_TYPE_PNG     "image/png"                 //!< Portable Network Graphics
#define REPO_MEDIA_TYPE_PBM     "image/x-portable-bitmap"	//!< Portable Bitmap
#define REPO_MEDIA_TYPE_PGM     "image/x-portable-graymap"	//!< Portable Graymap
#define REPO_MEDIA_TYPE_PPM     "image/x-portable-pixmap"	//!< Portable Pixmap
#define REPO_MEDIA_TYPE_XBM     "image/x-xbitmap"           //!< X11 Bitmap
#define REPO_MEDIA_TYPE_XPM     "image/x-xpixmap"           //!< X11 Pixmap
#define REPO_MEDIA_TYPE_SVG     "image/svg+xml"             //!< Scalable Vector Graphics

//------------------------------------------------------------------------------

#endif // REPO_CORE_GLOBAL_H
