/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- SeparatorEntry.h

Abstract:
- An entry in the "new tab" dropdown menu that represents all profiles
    that were not included explicitly elsewhere

Author(s):
- Floris Westerman - August 2022

--*/
#pragma once

#include "ProjectCollectionEntry.h"
#include "ProjectRemainingEntry.g.h"

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    struct ProjectRemainingEntry : ProjectRemainingEntryT<ProjectRemainingEntry, ProjectCollectionEntry>
    {
    public:
        ProjectRemainingEntry() noexcept;

        Model::NewProjectMenuEntry Copy() const override;

        static com_ptr<NewProjectMenuEntry> FromJson(const Json::Value& json);
    };
}

namespace winrt::Microsoft::Terminal::Settings::Model::factory_implementation
{
    BASIC_FACTORY(ProjectRemainingEntry);
}
