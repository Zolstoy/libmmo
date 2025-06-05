#pragma once

#include <expected>
#include <functional>
#include <string>

#ifdef HYPERBLOCK_WITH_ASIO
 #include <boost/asio.hpp>
 #include <boost/asio/io_context.hpp>
 #include <boost/asio/ssl.hpp>
#endif

#include "config.hpp"

#include "database.hpp"
#include "error.hpp"
#include "event.hpp"

namespace hyper_block {

class HYPERBLOCK_API instance
{
   public:
    friend struct inner;

   private:
    void* inner_;
    short port_;

    bool     is_running_;
    database database_;

   public:
#ifdef HYPERBLOCK_WITH_ASIO
    instance(boost::asio::io_context& io_context, std::string const& world_name, short port,
             std::string const& cert_pem, std::string const& key_pem,
             std::function<user_callback_proto>&& step_callback) noexcept;
#else
    instance(std::string const& world_name, short port, std::string const& cert_pem, std::string const& key_pem,
             std::function<user_callback_proto>&& step_callback) noexcept;
#endif

   public:
#ifdef HYPERBLOCK_WITH_ASIO
    std::expected<short, error> run_async() noexcept;
#else
   private:
    std::expected<short, error> run_async() noexcept;

   public:
    std::expected<std::tuple<>, error> run() noexcept;
#endif

   private:
    void do_accept();
};

}   // namespace hyper_block
