/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- SeparatorEntry.h

Abstract:
- A separator entry in the "new tab" dropdown menu

Author(s):
- Floris Westerman - August 2022

--*/
#pragma once

#include "NewProjectMenuEntry.h"
#include "ProjectSeparatorEntry.g.h"

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    struct ProjectSeparatorEntry : ProjectSeparatorEntryT<ProjectSeparatorEntry, NewProjectMenuEntry>
    {
    public:
        ProjectSeparatorEntry() noexcept;

        Model::NewProjectMenuEntry Copy() const override;

        static com_ptr<NewProjectMenuEntry> FromJson(const Json::Value& json);
    };
}

namespace winrt::Microsoft::Terminal::Settings::Model::factory_implementation
{
    BASIC_FACTORY(ProjectSeparatorEntry);
}
