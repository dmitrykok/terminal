// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "NewProjectMenuEntry.h"
#include "JsonUtils.h"
#include "TerminalSettingsSerializationHelpers.h"
#include "ProjectSeparatorEntry.h"
#include "ProjectFolderEntry.h"
#include "ProjectEntry.h"
#include "ProjectActionEntry.h"
#include "ProjectRemainingEntry.h"
#include "ProjectMatchEntry.h"

#include "NewProjectMenuEntry.g.cpp"

using namespace Microsoft::Terminal::Settings::Model;
using NewProjectMenuEntryType = winrt::Microsoft::Terminal::Settings::Model::NewProjectMenuEntryType;

static constexpr std::string_view TypeKey{ "type" };

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    NewProjectMenuEntry::NewProjectMenuEntry(const NewProjectMenuEntryType type) noexcept :
        _Type{ type }
    {
    }

    // This method will be overridden by the subclasses, which will then call this
    // parent implementation for a "base" json object.
    Json::Value NewProjectMenuEntry::ToJson() const
    {
        Json::Value json{ Json::ValueType::objectValue };

        JsonUtils::SetValueForKey(json, TypeKey, _Type);

        return json;
    }

    // Deserialize the JSON object based on the given type. We use the map from above for that.
    winrt::com_ptr<NewProjectMenuEntry> NewProjectMenuEntry::FromJson(const Json::Value& json)
    {
        const auto type = JsonUtils::GetValueForKey<NewProjectMenuEntryType>(json, TypeKey);

        switch (type)
        {
        case NewProjectMenuEntryType::ProjectSeparator:
            return ProjectSeparatorEntry::FromJson(json);
        case NewProjectMenuEntryType::ProjectFolder:
            return ProjectFolderEntry::FromJson(json);
        case NewProjectMenuEntryType::Project:
            return ProjectEntry::FromJson(json);
        case NewProjectMenuEntryType::ProjectRemaining:
            return ProjectRemainingEntry::FromJson(json);
        case NewProjectMenuEntryType::ProjectMatch:
            return ProjectMatchEntry::FromJson(json);
        case NewProjectMenuEntryType::ProjectAction:
            return ProjectActionEntry::FromJson(json);
        default:
            return nullptr;
        }
    }
}
