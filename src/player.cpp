#include "include/hyperblock/player.hpp"

namespace hyper_block {

player::player(float pos_x, float pos_y)
    : pos_x_(pos_x)
    , pos_y_(pos_y)
{}

float
player::get_pos_x() const
{
    return pos_x_;
}

float
player::get_pos_y() const
{
    return pos_y_;
}

}   // namespace hyper_block
