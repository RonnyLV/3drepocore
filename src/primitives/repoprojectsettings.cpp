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

#include "repoprojectsettings.h"

repo::core::RepoProjectSettings::~RepoProjectSettings() {}

std::string repo::core::RepoProjectSettings::getPermissionsString() const
{
    unsigned short octal = getPermissionsOctal();
    std::stringstream ss;
    ss << getRWX(octal,
                 core::RepoPermissionsBitMask::OWNER_READ,
                 core::RepoPermissionsBitMask::OWNER_WRITE,
                 core::RepoPermissionsBitMask::OWNER_EXECUTE);
    ss << getRWX(octal,
                 core::RepoPermissionsBitMask::GROUP_READ,
                 core::RepoPermissionsBitMask::GROUP_WRITE,
                 core::RepoPermissionsBitMask::GROUP_EXECUTE);
    ss << getRWX(octal,
                 core::RepoPermissionsBitMask::PUBLIC_READ,
                 core::RepoPermissionsBitMask::PUBLIC_WRITE,
                 core::RepoPermissionsBitMask::PUBLIC_EXECUTE);
    return ss.str();
}

std::string repo::core::RepoProjectSettings::getPermissionsOctalString() const
{
    std::stringstream ss;
    if (hasField(REPO_LABEL_PERMISSIONS))
    {
        std::vector<mongo::BSONElement> arr = getField(REPO_LABEL_PERMISSIONS).Array();
        for (unsigned int i = 0; i < arr.size(); ++i)
            ss << arr[i].Int();
    }
    return ss.str();
}

unsigned short repo::core::RepoProjectSettings::getPermissionsOctal() const
{
    return stringToOctal(getPermissionsOctalString());
}

std::vector<std::string> repo::core::RepoProjectSettings::getUsers() const
{
    std::vector<std::string> users;
    if (hasField(REPO_LABEL_USERS))
    {
        std::vector<mongo::BSONElement> arr = getField(REPO_LABEL_USERS).Array();
        users.resize(arr.size());
        for (unsigned int i = 0; i < arr.size(); ++i)
            users[i] = arr[i].String();
    }
    return users;
}

unsigned short repo::core::RepoProjectSettings::stringToOctal(const string &value)
{
    std::string octal = "0x";
    for (int i = 0; i < 4 - value.size(); ++i)
        octal += "0";
    octal += value;
    return (unsigned short) std::strtoul(octal.c_str(), NULL, 0);
}

std::string repo::core::RepoProjectSettings::getRWX(
        unsigned short octal,
        unsigned short rMask,
        unsigned short wMask,
        unsigned short xMask)
{
    std::stringstream ss;
    ss << (((octal & rMask) == rMask) ? "r" : "-");
    ss << (((octal & wMask) == wMask) ? "w" : "-");
    ss << (((octal & xMask) == xMask) ? "x" : "-");
    return ss.str();
}