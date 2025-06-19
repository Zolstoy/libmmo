#include <expected>
#include <future>
#include <print>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/beast.hpp>

#include <mmo/error.hpp>
#include <mmo/instance.hpp>
#include <mmo/protocol.hpp>
#include <mmo/tracing.hpp>

#include <cereal/archives/json.hpp>
#include <gtest/gtest.h>

#include "mmo/event.hpp"

#include "common.hpp"

using namespace boost;

// struct result {
//     struct no_error {
//     };
//     struct bad_event_type {
//     };
//     struct nickname_dont_match {
//     };
//     struct password_dont_match {
//     };
// };

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
        mmo::init_traces();
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
    std::println("[test] Getting instance...");
    auto instance = get_instance([](mmo::event &&event) -> void { throw std::runtime_error("from tests 123"); });
    std::println("[test] Launching instance...");
    auto future_result = std::async([i = std::move(instance)] mutable { return i.run(); });
    std::println("[test] Getting socket...");
    auto client_socket = get_socket();

    // asio::ip::tcp::resolver resolver(this->ioc);
    // auto                    results = resolver.resolve("localhost", "2456");

    std::println("[test] Connecting...");
    client_socket.next_layer().connect(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 2456));
    std::println("[test] Handshaking...");
    client_socket.handshake(boost::asio::ssl::stream_base::client);

    auto result = future_result.get();
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(static_cast<size_t>(mmo::error_code::error_not_implemented), result.error().index());
}
