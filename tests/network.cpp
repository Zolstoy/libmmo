#include <expected>
#include <future>
#include <thread>

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
#include <spdlog/spdlog.h>

#include "mmo/event.hpp"

#include "boost/system/detail/error_code.hpp"
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
    // asio::io_context        ioc_client;
    asio::ip::tcp::resolver resolver;

    network_test()
        : ssl_context(asio::ssl::context::sslv23_client)
        , resolver(ioc)
    {
        // ssl_context.set_verify_mode(asio::ssl::verify_none);
        ssl_context.add_certificate_authority(asio::buffer(CA_CERT.data(), CA_CERT.size()));
        // ssl_context.set_options(boost::asio::ssl::context::default_workarounds);
        ssl_context.set_verify_mode(asio::ssl::verify_peer);
        // asio::ip::tcp::resolver::results_type endpoints = resolver.resolve("localhost", "2456");
        // mmo::init_traces();
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
    spdlog::info("[test] Getting instance...");
    auto instance = get_instance([](mmo::event &&event) -> void {
        spdlog::info("[test] Inside handler");
        // throw std::runtime_error("from tests 123");
    });
    spdlog::info("[test] Launching instance...");
    std::promise<std::expected<std::tuple<>, mmo::error>> pro;
    std::future<std::expected<std::tuple<>, mmo::error>>  fut = pro.get_future();
    std::thread                                           t([&] { pro.set_value(instance.run()); });

    spdlog::info("[test] Getting socket...");
    auto client_socket = get_socket();

    asio::ip::tcp::resolver resolver(this->ioc);
    auto                    results = resolver.resolve("localhost", "2456");

    spdlog::info("[test] Connecting...");
    // client_socket.next_layer().connect(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 2456));
    boost::system::error_code ec;
    ec = client_socket.next_layer().connect(results.begin()->endpoint(), ec);
    if (ec)
    {
        spdlog::error("[test] Error connecting: {}", ec.message());
        ASSERT_FALSE(ec);
    }
    spdlog::info("[test] Handshaking...");
    client_socket.handshake(boost::asio::ssl::stream_base::client);

    t.join();
    auto result = fut.get();

    // ASSERT_FALSE(result.has_value());
    // ASSERT_EQ(static_cast<size_t>(mmo::error_code::error_not_implemented), result.error().index());
}

int
main(int argc, char **argv)
{
    mmo::init_traces();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
