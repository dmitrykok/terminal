#include "pch.h"
#include "SharedContext.h"
#include "SharedContext.g.cpp"

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    std::shared_ptr<SharedContext> SharedContext::_singletonInstance = nullptr;
    std::mutex SharedContext::_mutex;

    Model::SharedContext SharedContext::Instance()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // Ensure only one instance exists
        if (!_singletonInstance)
        {
            _singletonInstance = std::make_shared<SharedContext>();
        }

        return *_singletonInstance; // Return by value as WinRT requires
    }

    void SharedContext::SetLocalStateOverride(const winrt::hstring& path)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _localStateOverride = winrt::to_string(path);
    }

    winrt::hstring SharedContext::GetLocalStateOverride()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _localStateOverride.has_value() ? winrt::to_hstring(_localStateOverride.value()) : L"";
    }
}
