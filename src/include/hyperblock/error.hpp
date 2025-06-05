#pragma once

#include <string>
#include <variant>

namespace hyper_block {

enum class error_code
{
    error_not_implemented,
    invalid_certificate,
    invalid_private_key,
    instance_already_running,
    acceptor_failed
};

struct error_not_implemented {
};

struct invalid_certificate {
    std::string message;

    invalid_certificate(std::string const& message)
        : message(message)
    {}
};

struct invalid_private_key {
    std::string message;

    invalid_private_key(std::string const& message)
        : message(message)
    {}
};

struct instance_already_running {
};
struct acceptor_failed {
};

using error = std::variant<error_not_implemented, invalid_certificate, invalid_private_key, instance_already_running,
                           acceptor_failed>;

}   // namespace hyper_block
