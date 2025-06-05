#pragma once

#include <string>

namespace hyper_block {
namespace protocol {

struct next_message {
    int kind;

    std::string get_json() const;
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(kind);
    }
};

struct authentication {
    std::string nickname;
    std::string password;

    std::string get_json() const;
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(nickname, password);
    }
};

}   // namespace protocol
}   // namespace hyper_block
