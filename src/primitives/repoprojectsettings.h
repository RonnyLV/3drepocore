/**
 *  Copyright (C) 2015 3D Repo Ltd
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


#ifndef REPO_PROJECT_SETTINGS_H
#define REPO_PROJECT_SETTINGS_H

#include "repobson.h"
#include "../repocoreglobal.h"

namespace repo {
namespace core {

class REPO_CORE_EXPORT RepoProjectSettings : public RepoBSON
{

public:

    RepoProjectSettings() : RepoBSON() {}

    RepoProjectSettings(const mongo::BSONObj &obj) : RepoBSON(obj) {}

    ~RepoProjectSettings();

    //--------------------------------------------------------------------------

    //! Returns a new full (and owned) copy of the object.
    inline RepoProjectSettings copy() const { return RepoProjectSettings(RepoBSON::copy()); }

    //! Returns the project name.
    std::string getProject() const
    { return hasField(REPO_LABEL_ID) ? getField(REPO_LABEL_ID).String() : std::string(); }

    //! Returns description of the project.
    std::string getDescription() const
    { return hasField(REPO_LABEL_DESCRIPTION) ? getField(REPO_LABEL_DESCRIPTION).String() : std::string(); }

    //! Returns owner of the project.
    std::string getOwner() const
    { return hasField(REPO_LABEL_OWNER) ? getField(REPO_LABEL_OWNER).String() : std::string(); }

    //! Returns group of the project.
    std::string getGroup() const
    { return hasField(REPO_LABEL_GROUP) ? getField(REPO_LABEL_GROUP).String() : std::string(); }

    //! Returns unix-style permissions of the project, eg "0777".
    std::string getPermissionsString() const;

    //! Returns type of the project.
    std::string getType() const
    { return hasField(REPO_LABEL_TYPE) ? getField(REPO_LABEL_TYPE).String() : std::string(); }

    //! Returns users of the project.
    std::vector<std::string> getUsers() const;

}; // end class

} // end namespace core
} // end namespace repo

#endif // REPO_PROJECT_SETTINGS_H
