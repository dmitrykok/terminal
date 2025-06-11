// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "ProjectActionEntry.h"
#include "JsonUtils.h"

#include "ProjectActionEntry.g.cpp"

using namespace Microsoft::Terminal::Settings::Model;

static constexpr std::string_view ActionIdKey{ "id" };
static constexpr std::string_view IconKey{ "icon" };

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{

    ProjectActionEntry::ProjectActionEntry() noexcept :
        ProjectActionEntryT<ProjectActionEntry, NewProjectMenuEntry>(NewProjectMenuEntryType::ProjectAction)
    {
    }

    Json::Value ProjectActionEntry::ToJson() const
    {
        auto json = NewProjectMenuEntry::ToJson();

        JsonUtils::SetValueForKey(json, ActionIdKey, _ActionId);
        JsonUtils::SetValueForKey(json, IconKey, _Icon);

        return json;
    }

    winrt::com_ptr<NewProjectMenuEntry> ProjectActionEntry::FromJson(const Json::Value& json)
    {
        auto entry = winrt::make_self<ProjectActionEntry>();

        JsonUtils::GetValueForKey(json, ActionIdKey, entry->_ActionId);
        JsonUtils::GetValueForKey(json, IconKey, entry->_Icon);

        return entry;
    }

    Model::NewProjectMenuEntry ProjectActionEntry::Copy() const
    {
        auto entry = winrt::make_self<ProjectActionEntry>();
        entry->_ActionId = _ActionId;
        entry->_Icon = _Icon;
        return *entry;
    }
}
