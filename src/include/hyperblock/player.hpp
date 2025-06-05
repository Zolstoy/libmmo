#pragma once

#include "config.hpp"

namespace hyper_block {

class HYPERBLOCK_API player
{
   private:
    float pos_x_;
    float pos_y_;

   public:
    player(float pos_x, float pos_y);

   public:
    float get_pos_x() const;
    float get_pos_y() const;
};

}   // namespace hyper_block
