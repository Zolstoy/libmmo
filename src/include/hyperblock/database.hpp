#pragma once

#include "config.hpp"

#include <expected>

#include "error.hpp"
#include "player.hpp"

namespace hyper_block {

class HYPERBLOCK_API database
{
   private:
    void* inner_;
    float init_pos_x_;
    float init_pos_y_;

   public:
    database(std::string const& world_name);

   public:
    std::expected<std::tuple<>, error> save_user(std::string const& nickname, std::string const& password);
    std::expected<player, error>       load_user_player(std::string const& nickname, std::string const& password);

    void set_init_position(float pos_x, float pos_y);
};

}   // namespace hyper_block
