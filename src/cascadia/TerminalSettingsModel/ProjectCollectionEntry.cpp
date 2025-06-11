// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "ProjectCollectionEntry.h"
#include "JsonUtils.h"

#include "ProjectCollectionEntry.g.cpp"

using namespace Microsoft::Terminal::Settings::Model;
using namespace winrt::Microsoft::Terminal::Settings::Model::implementation;

ProjectCollectionEntry::ProjectCollectionEntry(const NewProjectMenuEntryType type) noexcept :
    ProjectCollectionEntryT<ProjectCollectionEntry, NewProjectMenuEntry>(type)
{
}
