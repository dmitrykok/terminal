/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- ActionEntry.h

Abstract:
- An action entry in the "new tab" dropdown menu

Author(s):
- Pankaj Bhojwani - May 2024

--*/
#pragma once

#include "NewProjectMenuEntry.h"
#include "ProjectActionEntry.g.h"

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    struct ProjectActionEntry : ProjectActionEntryT<ProjectActionEntry, NewProjectMenuEntry>
    {
    public:
        ProjectActionEntry() noexcept;

        Model::NewProjectMenuEntry Copy() const;

        Json::Value ToJson() const override;
        static com_ptr<NewProjectMenuEntry> FromJson(const Json::Value& json);

        WINRT_PROPERTY(winrt::hstring, ActionId);
        WINRT_PROPERTY(winrt::hstring, Icon);
    };
}

namespace winrt::Microsoft::Terminal::Settings::Model::factory_implementation
{
    BASIC_FACTORY(ProjectActionEntry);
}
