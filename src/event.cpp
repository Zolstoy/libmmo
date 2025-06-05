#include "include/mmo/event.hpp"

namespace mmo {
namespace events {

MMO_API
read::read(char const* message)
    : message(message)
{}

MMO_API
auth::auth(char const* nickname, char const* password)
    : nickname(nickname)
    , password(password)
{}

}   // namespace events

}   // namespace mmo
