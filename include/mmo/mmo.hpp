#pragma once

#include "config.hpp"

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <vector>


namespace mmo {

enum class order
{
    keep_going = 0,
    stop       = 1,
};

class player
{
   public:
    std::queue<std::string> in;
    std::queue<std::string> out;
};

using game_cycle_proto = order(float, std::map<size_t, std::shared_ptr<player>> const &);
using game_cycle       = std::function<game_cycle_proto>;

MMO_API extern void start(game_cycle &&cycle, std::vector<uint8_t> const &cert_pem, std::vector<uint8_t> const &key_pem,
                          unsigned short tick_in_ms = 250, unsigned short port = 8685);

}   // namespace mmo
