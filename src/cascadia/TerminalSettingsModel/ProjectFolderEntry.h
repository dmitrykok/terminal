/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- FolderEntry.h

Abstract:
- A folder entry in the "new tab" dropdown menu,

Author(s):
- Floris Westerman - August 2022

--*/
#pragma once

#include "pch.h"
#include "NewProjectMenuEntry.h"
#include "ProjectFolderEntry.g.h"

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    struct ProjectFolderEntry : ProjectFolderEntryT<ProjectFolderEntry, NewProjectMenuEntry>
    {
    public:
        ProjectFolderEntry() noexcept;
        explicit ProjectFolderEntry(const winrt::hstring& name) noexcept;

        Model::NewProjectMenuEntry Copy() const override;

        Json::Value ToJson() const override;
        static com_ptr<NewProjectMenuEntry> FromJson(const Json::Value& json);

        // In JSON, we can set arbitrarily many profiles or nested profiles, that might not all
        // be rendered; for example, when a profile entry is invalid, or when a folder is empty.
        // Therefore, we will store the JSON entries list internally, and then expose only the
        // entries to be rendered to WinRT.
        winrt::Windows::Foundation::Collections::IVector<Model::NewProjectMenuEntry> Entries() const;

        WINRT_PROPERTY(winrt::hstring, Name);
        WINRT_PROPERTY(winrt::hstring, Icon);
        WINRT_PROPERTY(ProjectFolderEntryInlining, Inlining, ProjectFolderEntryInlining::Never);
        WINRT_PROPERTY(bool, AllowEmpty, false);
        WINRT_PROPERTY(winrt::Windows::Foundation::Collections::IVector<Model::NewProjectMenuEntry>, RawEntries);
    };
}

namespace winrt::Microsoft::Terminal::Settings::Model::factory_implementation
{
    BASIC_FACTORY(ProjectFolderEntry);
}
