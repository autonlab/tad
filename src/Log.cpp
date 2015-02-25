#include "srl/Log.hpp"

namespace al { namespace srl
{
    const std::vector<Log::LogLevel> Log::AllLevels =
        { Log::Info, Log::Warning, Log::Error, Log::Debug };
    const std::vector<Log::LogLevel> Log::NotErrors =
        { Log::Info, Log::Warning, Log::Debug };
} }
