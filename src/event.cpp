#include "include/hyperblock/event.hpp"

namespace hyper_block {
namespace events {

HYPERBLOCK_API
read::read(char const* message)
    : message(message)
{}

HYPERBLOCK_API
auth::auth(char const* nickname, char const* password)
    : nickname(nickname)
    , password(password)
{}

}   // namespace events

}   // namespace hyper_block
