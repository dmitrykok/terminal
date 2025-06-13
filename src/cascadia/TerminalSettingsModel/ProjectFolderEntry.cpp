// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "ProjectFolderEntry.h"
#include "JsonUtils.h"
#include "TerminalSettingsSerializationHelpers.h"

#include "ProjectFolderEntry.g.cpp"

using namespace Microsoft::Terminal::Settings::Model;
using namespace winrt::Windows::Foundation::Collections;

static constexpr std::string_view NameKey{ "name" };
static constexpr std::string_view IconKey{ "icon" };
static constexpr std::string_view EntriesKey{ "entries" };
static constexpr std::string_view InliningKey{ "inline" };
static constexpr std::string_view AllowEmptyKey{ "allowEmpty" };

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    ProjectFolderEntry::ProjectFolderEntry() noexcept :
        ProjectFolderEntry{ winrt::hstring{} }
    {
    }

    ProjectFolderEntry::ProjectFolderEntry(const winrt::hstring& name) noexcept :
        ProjectFolderEntryT<ProjectFolderEntry, NewProjectMenuEntry>(NewProjectMenuEntryType::ProjectFolder),
        _Name{ name }
    {
    }

    Json::Value ProjectFolderEntry::ToJson() const
    {
        auto json = NewProjectMenuEntry::ToJson();

        JsonUtils::SetValueForKey(json, NameKey, _Name);
        JsonUtils::SetValueForKey(json, IconKey, _Icon);
        JsonUtils::SetValueForKey(json, EntriesKey, _RawEntries);
        JsonUtils::SetValueForKey(json, InliningKey, _Inlining);
        JsonUtils::SetValueForKey(json, AllowEmptyKey, _AllowEmpty);

        return json;
    }

    winrt::com_ptr<NewProjectMenuEntry> ProjectFolderEntry::FromJson(const Json::Value& json)
    {
        auto entry = winrt::make_self<ProjectFolderEntry>();

        JsonUtils::GetValueForKey(json, NameKey, entry->_Name);
        JsonUtils::GetValueForKey(json, IconKey, entry->_Icon);
        JsonUtils::GetValueForKey(json, EntriesKey, entry->_RawEntries);
        JsonUtils::GetValueForKey(json, InliningKey, entry->_Inlining);
        JsonUtils::GetValueForKey(json, AllowEmptyKey, entry->_AllowEmpty);

        return entry;
    }

    // A FolderEntry should only expose the entries to actually render to WinRT,
    // to keep the logic for collapsing/expanding more centralised.
    IVector<Model::NewProjectMenuEntry> ProjectFolderEntry::Entries() const
    {
        // We filter the full list of entries from JSON to just include the
        // non-empty ones.
        IVector<Model::NewProjectMenuEntry> result{ winrt::single_threaded_vector<Model::NewProjectMenuEntry>() };
        if (_RawEntries == nullptr)
        {
            return result;
        }

        for (const auto& entry : _RawEntries)
        {
            if (entry == nullptr)
            {
                continue;
            }

            switch (entry.Type())
            {
            case NewProjectMenuEntryType::Invalid:
                continue;

            // A profile is filtered out if it is not valid, so if it was not resolved
            case NewProjectMenuEntryType::Project:
            {
                const auto projectEntry = entry.as<Model::ProjectEntry>();
                if (projectEntry.Project() == nullptr)
                {
                    continue;
                }
                break;
            }

            // Any profile collection is filtered out if there are no results
            case NewProjectMenuEntryType::ProjectRemaining:
            case NewProjectMenuEntryType::ProjectMatch:
            {
                const auto projectCollectionEntry = entry.as<Model::ProjectCollectionEntry>();
                if (projectCollectionEntry.Projects().Size() == 0)
                {
                    continue;
                }
                break;
            }

            // A folder is filtered out if it has an effective size of 0 (calling
            // this filtering method recursively), and if it is not allowed to be
            // empty, or if it should auto-inline.
            case NewProjectMenuEntryType::ProjectFolder:
            {
                const auto folderEntry = entry.as<Model::ProjectFolderEntry>();
                if (folderEntry.Entries().Size() == 0 && (!folderEntry.AllowEmpty() || folderEntry.Inlining() == ProjectFolderEntryInlining::Auto))
                {
                    continue;
                }
                break;
            }
            }

            result.Append(entry);
        }

        return result;
    }

    Model::NewProjectMenuEntry ProjectFolderEntry::Copy() const
    {
        auto entry = winrt::make_self<ProjectFolderEntry>();
        entry->_Name = _Name;
        entry->_Icon = _Icon;
        entry->_Inlining = _Inlining;
        entry->_AllowEmpty = _AllowEmpty;

        if (_RawEntries)
        {
            entry->_RawEntries = winrt::single_threaded_vector<Model::NewProjectMenuEntry>();
            for (const auto& e : _RawEntries)
            {
                entry->_RawEntries.Append(get_self<NewProjectMenuEntry>(e)->Copy());
            }
        }
        return *entry;
    }
}
