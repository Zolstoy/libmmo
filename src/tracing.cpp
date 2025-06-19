#include "tracing.hpp"

#include <string>

#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace mmo {

class elapsed_formatter_flag : public spdlog::custom_flag_formatter
{
   private:
    std::chrono::steady_clock::time_point start_;

   public:
    elapsed_formatter_flag(std::chrono::steady_clock::time_point start)
        : start_(start)
    {}

   public:
    void format(const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t &dest) override
    {
        auto                         now     = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = now - start_;
        std::string some_txt                 = std::format("\033[38;2;100;20;200m{:10.3f}\033[39;49m", elapsed.count());
        dest.append(some_txt.data(), some_txt.data() + some_txt.size());
    }

    std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<elapsed_formatter_flag>(start_);
    }
};

class thread_formatter_flag : public spdlog::custom_flag_formatter
{
   public:
    void format(const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t &dest) override
    {
        auto              thread_id = std::this_thread::get_id();
        std::stringstream ss;
        ss << thread_id;
        std::string some_txt = std::format("\033[38;2;20;100;200m{:5}\033[39;49m", ss.str());
        dest.append(some_txt.data(), some_txt.data() + some_txt.size());
    }

    std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<thread_formatter_flag>();
    }
};

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
