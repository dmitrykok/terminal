#pragma once

#include "SharedContext.g.h"
#include <string>
#include <optional>
#include <mutex>

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    struct SharedContext : SharedContextT<SharedContext>
    {
    public:
        static Model::SharedContext Instance();

        void SetLocalStateOverride(const winrt::hstring& path);
        winrt::hstring GetLocalStateOverride();

    private:
        static std::shared_ptr<SharedContext> _singletonInstance;
        static std::mutex _mutex;

        std::optional<std::string> _localStateOverride;
    };
}

namespace winrt::Microsoft::Terminal::Settings::Model::factory_implementation
{
    BASIC_FACTORY(SharedContext);
}
