#pragma once

namespace hyper_block {

template <typename T>
concept Game = requires {
    {
        T::Player
    };
};

}   // namespace hyper_block
