#include "tracing.hpp"

#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace mmo {

void
init_traces()
{
    auto formatter = std::make_unique<spdlog::pattern_formatter>();
    auto now       = std::chrono::steady_clock::now();
    formatter->add_flag<mmo::elapsed_formatter_flag>('*', now);
    formatter->add_flag<mmo::thread_formatter_flag>('+');
    formatter->set_pattern("[%*][%+] %v");
    auto logger = spdlog::stdout_color_mt("console", spdlog::color_mode::always);
    logger->set_level(spdlog::level::trace);
    logger->set_formatter(std::move(formatter));

    spdlog::set_default_logger(logger);
}

}   // namespace mmo
