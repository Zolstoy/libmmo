#pragma once

#include "config.hpp"

#include <variant>

namespace hyper_block {

namespace events {

struct accept {
    constexpr static int value = 0;
};
struct handshake {
    constexpr static int value = 1;
};
struct upgrade {
    constexpr static int value = 2;
};
struct HYPERBLOCK_API read {
    constexpr static int value = 3;
    char const*          message;
    read(char const* message);
};
struct HYPERBLOCK_API auth {
    constexpr static int value = 4;
    char const*          nickname;
    char const*          password;
    auth(char const* nickname, char const* password);
};

}   // namespace events

using event = std::variant<events::accept, events::handshake, events::upgrade, events::read, events::auth>;
using user_callback_proto = void(event&&);

}   // namespace hyper_block
