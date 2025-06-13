// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "ProjectSeparatorEntry.h"
#include "JsonUtils.h"

#include "ProjectSeparatorEntry.g.cpp"

using namespace Microsoft::Terminal::Settings::Model;

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    ProjectSeparatorEntry::ProjectSeparatorEntry() noexcept :
        ProjectSeparatorEntryT<ProjectSeparatorEntry, NewProjectMenuEntry>(NewProjectMenuEntryType::ProjectSeparator)
    {
    }

    winrt::com_ptr<NewProjectMenuEntry> ProjectSeparatorEntry::FromJson(const Json::Value&)
    {
        return winrt::make_self<ProjectSeparatorEntry>();
    }

    Model::NewProjectMenuEntry ProjectSeparatorEntry::Copy() const
    {
        return winrt::make<ProjectSeparatorEntry>();
    }
}
