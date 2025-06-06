#pragma once

#include "config.hpp"

#include <expected>
#include <functional>
#include <string>

#include "database.hpp"
#include "error.hpp"
#include "event.hpp"

namespace mmo {

class MMO_API instance
{
   public:
    friend struct inner;

   private:
    void* inner_;
    short port_;

    bool     is_running_;
    database database_;

   public:
    instance(std::string const& world_name, short port, std::string const& cert_pem, std::string const& key_pem,
             std::function<user_callback_proto>&& step_callback) noexcept;

   private:
    std::expected<short, error> run_async() noexcept;

   public:
    std::expected<std::tuple<>, error> run() noexcept;

   private:
    void do_accept();
};

}   // namespace mmo
