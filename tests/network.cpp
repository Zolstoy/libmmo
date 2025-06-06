#include <expected>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/beast.hpp>

#include <mmo/instance.hpp>
#include <mmo/protocol.hpp>

#include <cereal/archives/json.hpp>
#include <gtest/gtest.h>

#include "mmo/event.hpp"

#include "boost/asio/ssl/context.hpp"
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
    asio::ssl::context ssl_context;

    network_test()
        : ssl_context(asio::ssl::context::sslv23)
    {
        ssl_context.set_verify_mode(asio::ssl::verify_none);
        ssl_context.add_certificate_authority(asio::buffer(CA_CERT.data(), CA_CERT.size()));
    }
};

TEST_F(network_test, case_01_accept)
{
    mmo::instance instance(get_random_instance_path(), 2456, CERT, KEY, [](mmo::event &&event) -> void {});
    auto          step1 = std::async([&] { return instance.run(); });

    asio::io_context ioc;

    asio::ip::tcp::resolver                  resolver(ioc);
    asio::ip::tcp::resolver::results_type    endpoints = resolver.resolve("localhost", "2456");
    asio::ssl::stream<asio::ip::tcp::socket> socket(ioc, get_ssl_context());
    ASSERT_THROW(step1.get(), result::no_error);
}
