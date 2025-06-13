// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "Project.h"
#include "JsonUtils.h"
#include "../../types/inc/Utils.hpp"

#include "TerminalSettingsSerializationHelpers.h"
#include "AppearanceConfig.h"
#include "FontConfig.h"

#include "Project.g.cpp"

#include <shellapi.h>

using namespace Microsoft::Terminal::Settings::Model;
using namespace winrt::Microsoft::Terminal::Settings::Model::implementation;
using namespace winrt::Microsoft::Terminal::Control;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::Foundation;
using namespace ::Microsoft::Console;

static constexpr std::string_view UpdatesKey{ "updates" };
static constexpr std::string_view NameKey{ "name" };
static constexpr std::string_view GuidKey{ "guid" };
static constexpr std::string_view SourceKey{ "source" };
static constexpr std::string_view HiddenKey{ "hidden" };
static constexpr std::string_view IconKey{ "icon" };

Project::Project(guid guid) noexcept :
	_Guid(guid)
{
}

winrt::com_ptr<Project> Project::CopySettings() const
{
	const auto project = winrt::make_self<Project>();
	const auto weakProject = winrt::make_weak<Model::Project>(*project);

	project->_Deleted = _Deleted;
	project->_Orphaned = _Orphaned;
	project->_Updates = _Updates;
	project->_Guid = _Guid;
	project->_Name = _Name;
	project->_Source = _Source;
	project->_Hidden = _Hidden;
	project->_Icon = _Icon;
	project->_Origin = _Origin;

#define PROJECT_SETTINGS_COPY(type, name, jsonKey, ...) \
	project->_##name = _##name;
#undef PROJECT_SETTINGS_COPY

	return project;
}

// Method Description:
// - Create a new instance of this class from a serialized JsonObject.
// Arguments:
// - json: an object which should be a serialization of a Project object.
// Return Value:
// - a new Project instance created from the values in `json`
winrt::com_ptr<winrt::Microsoft::Terminal::Settings::Model::implementation::Project> Project::FromJson(const Json::Value& json)
{
	auto result = winrt::make_self<Project>();
	result->LayerJson(json);
	return result;
}

// Method Description:
// - Layer values from the given json object on top of the existing properties
//   of this object. For any keys we're expecting to be able to parse in the
//   given object, we'll parse them and replace our settings with values from
//   the new json object. Properties that _aren't_ in the json object will _not_
//   be replaced.
// - Optional values in the project that are set to `null` in the json object
//   will be set to nullopt.
// Arguments:
// - json: an object which should be a partial serialization of a Project object.
// Return Value:
// <none>
void Project::LayerJson(const Json::Value& json)
{
	// Project-specific Settings
	JsonUtils::GetValueForKey(json, NameKey, _Name);
	JsonUtils::GetValueForKey(json, UpdatesKey, _Updates);
	JsonUtils::GetValueForKey(json, GuidKey, _Guid);

	// Make sure Source is before Hidden! We use that to exclude false positives from the settings logger!
	JsonUtils::GetValueForKey(json, SourceKey, _Source);
	JsonUtils::GetValueForKey(json, HiddenKey, _Hidden);
	JsonUtils::GetValueForKey(json, IconKey, _Icon);


#define PROJECT_SETTINGS_LAYER_JSON(type, name, jsonKey, ...) \
	JsonUtils::GetValueForKey(json, jsonKey, _##name);

#undef PROJECT_SETTINGS_LAYER_JSON

}

void Project::_FinalizeInheritance()
{
}

// Function Description:
// - Generates a unique guid for a project, given the name. For an given name, will always return the same GUID.
// Arguments:
// - name: The name to generate a unique GUID from
// Return Value:
// - a uuidv5 GUID generated from the given name.
winrt::guid Project::_GenerateGuidForProject(const std::wstring_view& name, const std::wstring_view& source) noexcept
{
	// If we have a _source, then we can from a dynamic project generator. Use
	// our source to build the namespace guid, instead of using the default GUID.

	const auto namespaceGuid = !source.empty() ?
								   Utils::CreateV5Uuid(RUNTIME_GENERATED_PROJECT_NAMESPACE_GUID, std::as_bytes(std::span{ source })) :
								   RUNTIME_GENERATED_PROJECT_NAMESPACE_GUID;

	// Always use the name to generate the temporary GUID. That way, across
	// reloads, we'll generate the same static GUID.
	return { Utils::CreateV5Uuid(namespaceGuid, std::as_bytes(std::span{ name })) };
}

// Method Description:
// - Create a new serialized JsonObject from an instance of this class
// Arguments:
// - <none>
// Return Value:
// - the JsonObject representing this instance
Json::Value Project::ToJson() const
{
	// Initialize the json with the appearance settings
	Json::Value json{ Json::ValueType::objectValue };

	// GH #9962:
	//   If the settings.json was missing, when we load the dynamic projects, they are completely empty.
	//   This caused us to serialize empty projects "{}" on accident.
	const auto writeBasicSettings{ !Source().empty() };

	// Project-specific Settings
	JsonUtils::SetValueForKey(json, NameKey, writeBasicSettings ? Name() : _Name);
	JsonUtils::SetValueForKey(json, GuidKey, writeBasicSettings ? Guid() : _Guid);
	JsonUtils::SetValueForKey(json, HiddenKey, writeBasicSettings ? Hidden() : _Hidden);
	JsonUtils::SetValueForKey(json, SourceKey, writeBasicSettings ? Source() : _Source);

	// Recall: Icon isn't actually a setting in the MTSM_PROJECT_SETTINGS. We
	// defined it manually in Project, so make sure we only serialize the Icon
	// if the user actually changed it here.
	JsonUtils::SetValueForKey(json, IconKey, _Icon);

#define PROJECT_SETTINGS_TO_JSON(type, name, jsonKey, ...) \
	JsonUtils::SetValueForKey(json, jsonKey, _##name);

#undef PROJECT_SETTINGS_TO_JSON

	return json;
}

// This is the implementation for and INHERITABLE_SETTING, but with one addition
// in the setter. We want to make sure to clear out our cached icon, so that we
// can re-evaluate it as it changes in the SUI.
void Project::Icon(const winrt::hstring& value)
{
	_evaluatedIcon = std::nullopt;
	_Icon = value;
}
winrt::hstring Project::Icon() const
{
	const auto val{ _getIconImpl() };
	return val ? *val : hstring{ L"\uE756" };
}

winrt::hstring Project::EvaluatedIcon()
{
	// We cache the result here, so we don't search the path for the exe every time.
	if (!_evaluatedIcon.has_value())
	{
		_evaluatedIcon = _evaluateIcon();
	}
	return *_evaluatedIcon;
}

winrt::hstring Project::_evaluateIcon() const
{
	// If the project has an icon, return it.
	if (!Icon().empty())
	{
		return Icon();
	}

	return hstring{ L"\uE756" };
}

bool Project::HasIcon() const
{
	return _Icon.has_value();
}

winrt::Microsoft::Terminal::Settings::Model::Project Project::IconOverrideSource()
{
	for (const auto& parent : _parents)
	{
		if (auto source{ parent->_getIconOverrideSourceImpl() })
		{
			return source;
		}
	}
	return nullptr;
}

void Project::ClearIcon()
{
	_Icon = std::nullopt;
	_evaluatedIcon = std::nullopt;
}

std::optional<winrt::hstring> Project::_getIconImpl() const
{
	if (_Icon)
	{
		return _Icon;
	}
	for (const auto& parent : _parents)
	{
		if (auto val{ parent->_getIconImpl() })
		{
			return val;
		}
	}
	return std::nullopt;
}

winrt::Microsoft::Terminal::Settings::Model::Project Project::_getIconOverrideSourceImpl() const
{
	if (_Icon)
	{
		return *this;
	}
	for (const auto& parent : _parents)
	{
		if (auto source{ parent->_getIconOverrideSourceImpl() })
		{
			return source;
		}
	}
	return nullptr;
}

void Project::LogSettingChanges(std::set<std::string>& changes, const std::string_view& context) const
{
	for (const auto& setting : _changeLog)
	{
		changes.emplace(fmt::format(FMT_COMPILE("{}.{}"), context, setting));
	}
}
