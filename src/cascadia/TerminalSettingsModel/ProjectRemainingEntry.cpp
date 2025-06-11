// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "ProjectRemainingEntry.h"
#include "NewProjectMenuEntry.h"
#include "JsonUtils.h"

#include "ProjectRemainingEntry.g.cpp"

using namespace Microsoft::Terminal::Settings::Model;

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    ProjectRemainingEntry::ProjectRemainingEntry() noexcept :
        ProjectRemainingEntryT<ProjectRemainingEntry, ProjectCollectionEntry>(NewProjectMenuEntryType::ProjectRemaining)
    {
    }

    winrt::com_ptr<NewProjectMenuEntry> ProjectRemainingEntry::FromJson(const Json::Value&)
    {
        return winrt::make_self<ProjectRemainingEntry>();
    }

    Model::NewProjectMenuEntry ProjectRemainingEntry::Copy() const
    {
        return winrt::make<ProjectRemainingEntry>();
    }
}
