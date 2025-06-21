#pragma once

#include "config.hpp"

#include <functional>
#include <string>
#include <thread>

#include "database.hpp"
#include "event.hpp"

namespace mmo {

class MMO_API instance
{
   public:
    friend struct inner;

   private:
    void*    inner_;
    short    port_;
    database database_;

   public:
    instance(std::string const& world_name, short port, std::string const& cert_pem, std::string const& key_pem,
             std::function<user_callback_proto>&& step_callback);
    ~instance();

   public:
    std::jthread launch();
    void         run();

   private:
    void do_accept();
};

}   // namespace mmo
