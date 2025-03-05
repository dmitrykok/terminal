// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "FileUtils.h"

#include <appmodel.h>
#include <shlobj.h>
#include <WtExeUtils.h>
#include <wil/resource.h>

static constexpr std::wstring_view UnpackagedSettingsFolderName{ L"Microsoft\\Windows Terminal\\" };
static constexpr std::wstring_view ReleaseSettingsFolder{ L"Packages\\Microsoft.WindowsTerminal_8wekyb3d8bbwe\\LocalState\\" };
static constexpr std::wstring_view PortableModeMarkerFile{ L".portable" };
static constexpr std::wstring_view PortableModeSettingsFolder{ L"settings" };

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

            if (!IsPackaged() && IsPortableMode())
            {
                std::filesystem::path modulePath{ wil::GetModuleFileNameW<std::wstring>(wil::GetModuleInstanceHandle()) };
                modulePath.replace_filename(PortableModeSettingsFolder);
                std::filesystem::create_directories(modulePath);
                return modulePath;
            }

            static std::filesystem::path envSettingsPath;
            wil::unique_hstring envSettings;
            wchar_t envPath[MAX_PATH];
            DWORD result = GetEnvironmentVariableW(L"WT_CUSTOM_LOCAL_STATE_PATH", envPath, MAX_PATH);
            if (result > 0 && result < MAX_PATH)
            {
                wchar_t* bufferStorage = nullptr;
                wil::unique_hstring_buffer theBuffer;
                THROW_IF_FAILED(::WindowsPreallocateStringBuffer(65, &bufferStorage, &theBuffer));
                THROW_IF_FAILED(::PathCchCombine(bufferStorage, 65, envPath, L""));
                THROW_IF_FAILED(wil::make_hstring_from_buffer_nothrow(wistd::move(theBuffer), &envSettings));
                envSettingsPath. = envSettings.get();
                //CustomSettingsFolder = std::filesystem::path{ envPath };
                //std::filesystem::create_directories(envSettingsPath);
                //return envSettingsPath;
            }

            wil::shared_cotaskmem_string localAppDataFolder;
            // KF_FLAG_FORCE_APP_DATA_REDIRECTION, when engaged, causes SHGet... to return
            // the new AppModel paths (Packages/xxx/RoamingState, etc.) for standard path requests.
            // Using this flag allows us to avoid Windows.Storage.ApplicationData completely.
            THROW_IF_FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_FORCE_APP_DATA_REDIRECTION, nullptr, &localAppDataFolder));

            std::filesystem::path parentDirectoryForSettingsFile{ localAppDataFolder.get() };
            //std::filesystem::path parentDirectoryForSettingsFile{ envPath };

            if (!IsPackaged())
            {
                parentDirectoryForSettingsFile /= UnpackagedSettingsFolderName;
            }

            if (envSettings.is_valid())
            {
                std::filesystem::path envSettingsPath{ envSettings.get() };
                parentDirectoryForSettingsFile = envSettingsPath;
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
            // Check if WT_CUSTOM_LOCAL_STATE_PATH is set (only once)
            char* envPath = nullptr;
            size_t len = 0;
            if (_dupenv_s(&envPath, &len, "WT_CUSTOM_LOCAL_STATE_PATH") == 0 && envPath != nullptr)
            {
                auto envSettingsPath = std::filesystem::path{ envPath };
                //free(envPath); // Free allocated memory
                // Create the directory if it doesn't exist
                std::filesystem::create_directories(envSettingsPath);

                return envSettingsPath;
            }

            wil::unique_cotaskmem_string localAppDataFolder;
            // We're using KF_FLAG_NO_PACKAGE_REDIRECTION to ensure that we always get the
            // user's actual local AppData directory.
            THROW_IF_FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_NO_PACKAGE_REDIRECTION, nullptr, &localAppDataFolder));

            // Returns a path like C:\Users\<username>\AppData\Local
            std::filesystem::path parentDirectoryForSettingsFile{ localAppDataFolder.get() };

            //Appending \Packages\Microsoft.WindowsTerminal_8wekyb3d8bbwe\LocalState to the settings path
            parentDirectoryForSettingsFile /= ReleaseSettingsFolder;

            if (!IsPackaged())
            {
                parentDirectoryForSettingsFile /= UnpackagedSettingsFolderName;
            }

            return parentDirectoryForSettingsFile;
        }();
        return baseSettingsPath;
    }
}
