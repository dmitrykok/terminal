/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- NewProjectMenuEntry.h

Abstract:
- An entry in the "new project" dropdown menu. These entries exist in a few varieties,
    such as separators, folders, or profile links.

Author(s):
- Floris Westerman - August 2022

--*/
#pragma once

#include "NewProjectMenuEntry.g.h"
#include "JsonUtils.h"

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    struct NewProjectMenuEntry : NewProjectMenuEntryT<NewProjectMenuEntry>
    {
    public:
        static com_ptr<NewProjectMenuEntry> FromJson(const Json::Value& json);
        virtual Json::Value ToJson() const;
        virtual Model::NewProjectMenuEntry Copy() const = 0;

        WINRT_PROPERTY(NewProjectMenuEntryType, Type, NewProjectMenuEntryType::Invalid);

        // We have a protected/hidden constructor so consumers cannot instantiate
        // this base class directly and need to go through either FromJson
        // or one of the subclasses.
    protected:
        explicit NewProjectMenuEntry(const NewProjectMenuEntryType type) noexcept;
    };
}

namespace Microsoft::Terminal::Settings::Model::JsonUtils
{
    using namespace winrt::Microsoft::Terminal::Settings::Model;

    template<>
    struct ConversionTrait<NewProjectMenuEntry>
    {
        NewProjectMenuEntry FromJson(const Json::Value& json)
        {
            const auto entry = implementation::NewProjectMenuEntry::FromJson(json);
            if (entry == nullptr)
            {
                return nullptr;
            }

            return *entry;
        }

        bool CanConvert(const Json::Value& json) const
        {
            return json.isObject();
        }

        Json::Value ToJson(const NewProjectMenuEntry& val)
        {
            return winrt::get_self<implementation::NewProjectMenuEntry>(val)->ToJson();
        }

        std::string TypeDescription() const
        {
            return "NewProjectMenuEntry";
        }
    };
}
