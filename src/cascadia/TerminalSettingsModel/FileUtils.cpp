// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "FileUtils.h"

#include <appmodel.h>
#include <shlobj.h>
#include <WtExeUtils.h>

#define ENV_WT_BASE_SETTINGS_PATH L"WT_BASE_SETTINGS_PATH"
#define ENV_WT_PROJECT_NAME L"WT_PROJECT_NAME"

static constexpr std::wstring_view UnpackagedSettingsFolderName{ L"Microsoft\\Windows Terminal\\" };
static constexpr std::wstring_view ReleaseSettingsFolder{ L"Packages\\Microsoft.WindowsTerminal_8wekyb3d8bbwe\\LocalState\\" };
static constexpr std::wstring_view PortableModeMarkerFile{ L".portable" };
static constexpr std::wstring_view PortableModeSettingsFolder{ L"settings" };
static constexpr std::wstring_view ProjectFolderPrefix{ L"Projects" };

namespace winrt::Microsoft::Terminal::Settings::Model
{
	// Returns a path like C:\Users\<username>\AppData\Local\Packages\<packagename>\LocalState
	// You can put your settings.json or state.json in this directory.
	bool IsPortableMode()
	{
		static auto portableMode = []() {
			std::filesystem::path modulePath{ wil::GetModuleFileNameW<std::wstring>(wil::GetModuleInstanceHandle()) };
			modulePath.replace_filename(PortableModeMarkerFile);
			return std::filesystem::exists(modulePath);
		}();
		return portableMode;
	}

	std::filesystem::path GetBaseSettingsPath()
	{
		static auto baseSettingsPath = []() {
			try
			{
				std::filesystem::path envSettingsPath{ wil::TryGetEnvironmentVariableW<std::wstring>(ENV_WT_BASE_SETTINGS_PATH) };
				if (!envSettingsPath.empty())
				{
					std::filesystem::create_directories(envSettingsPath);
					return envSettingsPath;
				}
			}
			CATCH_LOG()

			std::wstring envProjectName{ wil::TryGetEnvironmentVariableW<std::wstring>(ENV_WT_PROJECT_NAME) };

			if (!IsPackaged() && IsPortableMode())
			{
				std::filesystem::path modulePath{ wil::GetModuleFileNameW<std::wstring>(wil::GetModuleInstanceHandle()) };
				modulePath.replace_filename(PortableModeSettingsFolder);
				if (!envProjectName.empty())
				{
					modulePath /= ProjectFolderPrefix;
					modulePath /= envProjectName;
				}
				std::filesystem::create_directories(modulePath);
				return modulePath;
			}

			wil::unique_cotaskmem_string localAppDataFolder;
			// KF_FLAG_FORCE_APP_DATA_REDIRECTION, when engaged, causes SHGet... to return
			// the new AppModel paths (Packages/xxx/RoamingState, etc.) for standard path requests.
			// Using this flag allows us to avoid Windows.Storage.ApplicationData completely.
			THROW_IF_FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_FORCE_APP_DATA_REDIRECTION, nullptr, &localAppDataFolder));

			std::filesystem::path parentDirectoryForSettingsFile{ localAppDataFolder.get() };

			if (!envProjectName.empty())
			{
				parentDirectoryForSettingsFile /= ProjectFolderPrefix;
				parentDirectoryForSettingsFile /= envProjectName;
			}

			if (!IsPackaged())
			{
				parentDirectoryForSettingsFile /= UnpackagedSettingsFolderName;
			}

			// Create the directory if it doesn't exist
			std::filesystem::create_directories(parentDirectoryForSettingsFile);

			return parentDirectoryForSettingsFile;
		}();
		return baseSettingsPath;
	}

	// Returns a path like C:\Users\<username>\AppData\Local\Packages\Microsoft.WindowsTerminal_8wekyb3d8bbwe\LocalState
	// to the path of the stable release settings
	std::filesystem::path GetReleaseSettingsPath()
	{
		static std::filesystem::path baseSettingsPath = []() {
			try
			{
				std::filesystem::path envSettingsPath{ wil::TryGetEnvironmentVariableW<std::wstring>(ENV_WT_BASE_SETTINGS_PATH) };
				if (!envSettingsPath.empty())
				{
					std::filesystem::create_directories(envSettingsPath);
					return envSettingsPath;
				}
			}
			CATCH_LOG()

			std::wstring envProjectName{ wil::TryGetEnvironmentVariableW<std::wstring>(ENV_WT_PROJECT_NAME) };

			wil::unique_cotaskmem_string localAppDataFolder;
			// We're using KF_FLAG_NO_PACKAGE_REDIRECTION to ensure that we always get the
			// user's actual local AppData directory.
			THROW_IF_FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_NO_PACKAGE_REDIRECTION, nullptr, &localAppDataFolder));

			// Returns a path like C:\Users\<username>\AppData\Local
			std::filesystem::path parentDirectoryForSettingsFile{ localAppDataFolder.get() };

			//Appending \Packages\Microsoft.WindowsTerminal_8wekyb3d8bbwe\LocalState to the settings path
			parentDirectoryForSettingsFile /= ReleaseSettingsFolder;

			if (!envProjectName.empty())
			{
				parentDirectoryForSettingsFile /= ProjectFolderPrefix;
				parentDirectoryForSettingsFile /= envProjectName;
			}

			if (!IsPackaged())
			{
				parentDirectoryForSettingsFile /= UnpackagedSettingsFolderName;
			}

			return parentDirectoryForSettingsFile;
		}();
		return baseSettingsPath;
	}
}
