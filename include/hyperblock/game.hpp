#pragma once

namespace mmo {

template <typename T>
concept Game = requires {
    {
        T::Player
    };
};

}   // namespace mmo
