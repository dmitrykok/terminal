/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- FolderEntry.h

Abstract:
- A profile entry in the "new tab" dropdown menu, referring to
    a single profile.

Author(s):
- Floris Westerman - August 2022

--*/
#pragma once

#include "NewProjectMenuEntry.h"
#include "ProjectEntry.g.h"

#include "Project.h"

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    struct ProjectEntry : ProjectEntryT<ProjectEntry, NewProjectMenuEntry>
    {
    public:
        ProjectEntry() noexcept;
        explicit ProjectEntry(const winrt::hstring& project) noexcept;

        Model::NewProjectMenuEntry Copy() const override;

        Json::Value ToJson() const override;
        static com_ptr<NewProjectMenuEntry> FromJson(const Json::Value& json);

        // In JSON, only a profile name (guid or string) can be set;
        // but the consumers of this class would like to have direct access
        // to the appropriate Model::Profile. Therefore, we have a read-only
        // property ProfileName that corresponds to the JSON value, and
        // then CascadiaSettings::_resolveNewTabMenuProfiles() will populate
        // the Profile and ProfileIndex properties appropriately
        winrt::hstring ProjectName() const noexcept { return _ProjectName; };

        WINRT_PROPERTY(Model::Project, Project);
        WINRT_PROPERTY(int, ProjectIndex);
        WINRT_PROPERTY(winrt::hstring, Icon);

    private:
        winrt::hstring _ProjectName;
    };
}

namespace winrt::Microsoft::Terminal::Settings::Model::factory_implementation
{
    BASIC_FACTORY(ProjectEntry);
}
