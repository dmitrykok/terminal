/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- Project.hpp

Abstract:
- A Project acts as a single set of terminal settings. Many tabs or panes could
  exist side-by-side with different Projects simultaneously.
- Projects could also specify their appearance when unfocused, this is what
  the inheritance tree looks like for unfocused settings:

				+-------------------+
				|                   |
				|Project.defaults   |
				|                   |
				|DefaultAppearance  |
				|                   |
				+-------------------+
				   ^             ^
				   |             |
+------------------++           ++------------------+
|                   |           |                   |
|MyProject          |           |Project.defaults   |
|                   |           |                   |
|DefaultAppearance  |           |UnfocusedAppearance|
|                   |           |                   |
+-------------------+           +-------------------+
				   ^
				   |
+------------------++
|                   |
|MyProject          |
|                   |
|UnfocusedAppearance|
|                   |
+-------------------+


Author(s):
- Mike Griese - March 2019

--*/
#pragma once

#include "Project.g.h"
#include "IInheritable.h"
#include "MTSMSettings.h"

#include "JsonUtils.h"
#include <DefaultSettings.h>
#include "AppearanceConfig.h"
#include "FontConfig.h"

// fwdecl unittest classes
namespace SettingsModelUnitTests
{
	class DeserializationTests;
	class ProjectTests;
	class ColorSchemeTests;
	class KeyBindingsTests;
};
namespace TerminalAppUnitTests
{
	class DynamicProjectTests;
	class JsonTests;
};

using IEnvironmentVariableMap = winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::hstring>;

// GUID used for generating GUIDs at runtime, for Projects that did not have a
// GUID specified manually.
constexpr GUID RUNTIME_GENERATED_PROJECT_NAMESPACE_GUID = { 0x42439e5, 0x628d, 0x462b, { 0x84, 0xe1, 0xc, 0xd5, 0x42, 0x5c, 0xa6, 0x13 } };
;

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
	struct Project : ProjectT<Project>, IInheritable<Project>
	{
	public:
		Project() noexcept = default;
		Project(guid guid) noexcept;

		hstring ToString()
		{
			return Name();
		}
		winrt::com_ptr<Project> CopySettings() const;

		static com_ptr<Project> FromJson(const Json::Value& json);
		void LayerJson(const Json::Value& json);

		Json::Value ToJson() const;

		void _FinalizeInheritance() override;

		void LogSettingChanges(std::set<std::string>& changes, const std::string_view& context) const;

		// EvaluatedIcon depends on Icon. It allows us to grab the
		//   icon from an exe path.
		// As a result, we can't use the INHERITABLE_SETTING macro for Icon,
		//   as we manually have to set/unset _evaluatedIcon when Icon changes.
		winrt::hstring EvaluatedIcon();
		hstring Icon() const;
		void Icon(const hstring& value);
		bool HasIcon() const;
		Model::Project IconOverrideSource();
		void ClearIcon();

		WINRT_PROPERTY(bool, Deleted, false);
		WINRT_PROPERTY(bool, Orphaned, false);
		WINRT_PROPERTY(OriginTag, Origin, OriginTag::None);
		WINRT_PROPERTY(guid, Updates);

		// Nullable/optional settings

		// Settings that cannot be put in the macro because of how they are handled in ToJson/LayerJson
		INHERITABLE_SETTING(Model::Project, hstring, Name, L"Default");
		INHERITABLE_SETTING(Model::Project, hstring, Source);
		INHERITABLE_SETTING(Model::Project, bool, Hidden, false);
		INHERITABLE_SETTING(Model::Project, guid, Guid, _GenerateGuidForProject(Name(), Source()));

	public:
#define PROJECT_SETTINGS_INITIALIZE(type, name, jsonKey, ...) \
	INHERITABLE_SETTING_WITH_LOGGING(Model::Project, type, name, jsonKey, ##__VA_ARGS__)
#undef PROJECT_SETTINGS_INITIALIZE

	private:
		std::optional<hstring> _Icon{ std::nullopt };
		std::optional<winrt::hstring> _evaluatedIcon{ std::nullopt };
		std::set<std::string> _changeLog;

		static guid _GenerateGuidForProject(const std::wstring_view& name, const std::wstring_view& source) noexcept;

		winrt::hstring _evaluateIcon() const;
		std::optional<hstring> _getIconImpl() const;
		Model::Project _getIconOverrideSourceImpl() const;
	};
}

namespace winrt::Microsoft::Terminal::Settings::Model::factory_implementation
{
	BASIC_FACTORY(Project);
}
