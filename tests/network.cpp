#include <expected>
#include <future>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/beast.hpp>

#include <mmo/instance.hpp>
#include <mmo/protocol.hpp>

#include <cereal/archives/json.hpp>
#include <gtest/gtest.h>

#include "mmo/event.hpp"

#include "common.hpp"

using namespace boost;

struct result {
    struct no_error {
    };
    struct bad_event_type {
    };
    struct nickname_dont_match {
    };
    struct password_dont_match {
    };
};

struct network_test : public testing::Test {
    asio::ssl::context      ssl_context;
    asio::io_context        ioc;
    asio::ip::tcp::resolver resolver;

    network_test()
        : ssl_context(asio::ssl::context::sslv23)
        , resolver(ioc)
    {
        ssl_context.set_verify_mode(asio::ssl::verify_none);
        ssl_context.add_certificate_authority(asio::buffer(CA_CERT.data(), CA_CERT.size()));

        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve("localhost", "2456");
    }

    mmo::instance get_instance(std::function<mmo::user_callback_proto> &&func) const
    {
        return mmo::instance(get_random_instance_path(), 2456, CERT, KEY, std::move(func));
    }

    asio::ssl::stream<asio::ip::tcp::socket> get_socket()
    {
        return asio::ssl::stream<asio::ip::tcp::socket>(ioc, ssl_context);
    }
};

TEST_F(network_test, case_01_accept)
{
    auto instance      = get_instance([](mmo::event &&event) -> void {
        if (event.index() != mmo::events::accept::value)
            throw result::bad_event_type{};
        throw result::no_error{};
    });
    auto future_result = std::async([i = std::move(instance)] mutable { return i.run(); });
    auto client_socket = get_socket();

    std::expected<std::tuple<>, mmo::error> err;

    ASSERT_THROW(err = future_result.get(), result::no_error);
}
