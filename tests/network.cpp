#include <exception>
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

namespace result {

struct error_base : public std::exception {
};

struct no_error : public error_base {
};
struct bad_event_type : public error_base {
};
struct nickname_dont_match : public error_base {
};
struct password_dont_match : public error_base {
};

};   // namespace result

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
    auto                                                  instance = get_instance([](mmo::event &&event) -> void {
        if (event.index() != mmo::events::accept::value)
            throw result::bad_event_type{};
        throw result::no_error{};
    });
    std::promise<std::expected<std::tuple<>, mmo::error>> pro;
    std::future<std::expected<std::tuple<>, mmo::error>>  fut = pro.get_future();
    std::jthread                                          thrd([&] {
        try
        {
            auto result = instance.run();
            pro.set_value(result);
        } catch (...)
        {
            pro.set_exception(std::current_exception());
        }
    });

    auto client_socket = get_socket();

    asio::ip::tcp::resolver resolver(this->ioc);
    auto                    results = resolver.resolve("127.0.0.1", "2456");

    boost::system::error_code ec;
    ec = client_socket.next_layer().connect(results.begin()->endpoint(), ec);
    if (ec)
    {
        spdlog::error("[test] Error connecting: {}", ec.message());
        ASSERT_FALSE(ec);
    }
    std::expected<std::tuple<>, mmo::error> result;

    spdlog::info("[test] Getting results...");
    ASSERT_TRUE(fut.valid());

    ASSERT_THROW(result = fut.get(), result::no_error);
    spdlog::info("TEST ENDED. Joining thread...");
    thrd.join();
    spdlog::info("[test] Thread joined.");
}

int
main(int argc, char **argv)
{
    mmo::init_traces();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
