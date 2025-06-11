/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- ProjectMatchEntry.h

Abstract:
- An entry in the "new tab" dropdown menu that represents a collection
    of profiles that match a specified name, source, or command line.

Author(s):
- Floris Westerman - November 2022

--*/
#pragma once

#include "ProjectCollectionEntry.h"
#include "ProjectMatchEntry.g.h"
#include <til/regex.h>

// This macro defines the getter and setter for a regex property.
// The setter tries to instantiate the regex immediately and caches
// it if successful. If it fails, it sets a boolean flag to track that
// it failed.
#define DEFINE_MATCH_PROJECT_REGEX_PROPERTY(name)      \
public:                                                \
    hstring name() const noexcept                      \
    {                                                  \
        return _##name;                                \
    }                                                  \
    void name(const hstring& value) noexcept           \
    {                                                  \
        _##name = value;                               \
        _validateAndPopulate##name##Regex();           \
    }                                                  \
                                                       \
private:                                               \
    void _validateAndPopulate##name##Regex() noexcept; \
                                                       \
    hstring _##name;                                   \
    til::ICU::unique_uregex _##name##Regex;            \
    bool _invalid##name{ false };

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    struct ProjectMatchEntry : ProjectMatchEntryT<ProjectMatchEntry, ProjectCollectionEntry>
    {
    public:
        ProjectMatchEntry() noexcept;

        Model::NewProjectMenuEntry Copy() const override;

        Json::Value ToJson() const override;
        static com_ptr<NewProjectMenuEntry> FromJson(const Json::Value& json);

        bool ValidateRegexes() const;
        bool MatchesProfile(const Model::Profile& profile);

        DEFINE_MATCH_PROJECT_REGEX_PROPERTY(Name)
        DEFINE_MATCH_PROJECT_REGEX_PROPERTY(Commandline)
        DEFINE_MATCH_PROJECT_REGEX_PROPERTY(Source)
    };
}

namespace winrt::Microsoft::Terminal::Settings::Model::factory_implementation
{
    BASIC_FACTORY(ProjectMatchEntry);
}
