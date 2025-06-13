// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "ProjectEntry.h"
#include "JsonUtils.h"

#include "ProjectEntry.g.cpp"

using namespace Microsoft::Terminal::Settings::Model;

static constexpr std::string_view ProjectKey{ "project" };
static constexpr std::string_view IconKey{ "icon" };

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    ProjectEntry::ProjectEntry() noexcept :
        ProjectEntry{ winrt::hstring{} }
    {
    }

    ProjectEntry::ProjectEntry(const winrt::hstring& project) noexcept :
        ProjectEntryT<ProjectEntry, NewProjectMenuEntry>(NewProjectMenuEntryType::Project),
        _ProjectName{ project }
    {
    }

    Json::Value ProjectEntry::ToJson() const
    {
        auto json = NewProjectMenuEntry::ToJson();

        // We will now return a project reference to the JSON representation. Logic is
        // as follows:
        // - When Project is null, this is typically because an existing project menu entry
        //   in the JSON config is invalid (nonexistent or hidden project). Then, we store
        //   the original project string value as read from JSON, to improve portability
        //   of the settings file and limit modifications to the JSON.
        // - Otherwise, we always store the GUID of the project. This will effectively convert
        //   all name-matched projects from the settings file to GUIDs. This might be unexpected
        //   to some users, but is less error-prone and will continue to work when project
        //   names are changed.
        if (_Project == nullptr)
        {
            JsonUtils::SetValueForKey(json, ProjectKey, _ProjectName);
        }
        else
        {
            JsonUtils::SetValueForKey(json, ProjectKey, _Project.Guid());
        }
        JsonUtils::SetValueForKey(json, IconKey, _Icon);

        return json;
    }

    winrt::com_ptr<NewProjectMenuEntry> ProjectEntry::FromJson(const Json::Value& json)
    {
        auto entry = winrt::make_self<ProjectEntry>();

        JsonUtils::GetValueForKey(json, ProjectKey, entry->_ProjectName);
        JsonUtils::GetValueForKey(json, IconKey, entry->_Icon);

        return entry;
    }

    Model::NewProjectMenuEntry ProjectEntry::Copy() const
    {
        auto entry{ winrt::make_self<ProjectEntry>() };
        entry->_Project = _Project;
        entry->_ProjectIndex = _ProjectIndex;
        entry->_ProjectName = _ProjectName;
        entry->_Icon = _Icon;
        return *entry;
    }
}
