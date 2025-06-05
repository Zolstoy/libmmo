// #include <expected>

// #include <boost/asio.hpp>
// #include <boost/asio/io_context.hpp>
// #include <boost/asio/ssl.hpp>
// #include <boost/asio/ssl/stream.hpp>
// #include <boost/asio/ssl/stream_base.hpp>
// #include <boost/beast.hpp>

// #include <mmo/instance.hpp>
// #include <mmo/protocol.hpp>

// #include <cereal/archives/json.hpp>
// #include <gtest/gtest.h>

// #include "mmo/event.hpp"

// #include "common.hpp"

// using namespace boost;

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

// #define MMO_TESTS_PRELUDE(code)                                                                    \
//     asio::io_context   ioc_server;                                                                 \
//     mmo::instance      instance(ioc_server, get_random_instance_path(), 2456, CERT, KEY,           \
//                                 [](mmo::event &&event) -> void code);                              \
//     auto               port  = instance.run_async().value();                                       \
//     auto               step1 = std::async(std::launch::async, [&]() { return ioc_server.run(); }); \
//     asio::io_context   ioc_client;                                                                 \
//     asio::ssl::context ssl_context(asio::ssl::context::sslv23);                                    \
//     ssl_context.set_verify_mode(asio::ssl::verify_none);                                           \
//     ssl_context.add_certificate_authority(asio::buffer(CA_CERT.data(), CA_CERT.size()));           \
//     asio::ip::tcp::resolver                  resolver(ioc_client);                                 \
//     asio::ip::tcp::resolver::results_type    endpoints = resolver.resolve("localhost", "2456");    \
//     asio::ssl::stream<asio::ip::tcp::socket> socket(ioc_client, ssl_context)

// TEST(network, case_01_accept)
// {
//     MMO_TESTS_PRELUDE({
//         if (event.index() != mmo::events::accept::value)
//             throw result::bad_event_type{};
//         throw result::no_error{};
//     });

//     asio::connect(socket.next_layer(), endpoints);
//     ASSERT_THROW(step1.get(), result::no_error);
// }

// TEST(network, case_02_handshake)
// {
//     MMO_TESTS_PRELUDE({
//         if (event.index() == mmo::events::accept::value)
//             return;
//         if (event.index() != mmo::events::handshake::value)
//             throw result::bad_event_type{};
//         throw result::no_error{};
//     });

//     asio::connect(socket.next_layer(), endpoints);
//     socket.handshake(asio::ssl::stream_base::client);
//     ASSERT_THROW(step1.get(), result::no_error);
// }

// TEST(network, case_03_upgrade)
// {
//     MMO_TESTS_PRELUDE({
//         if (event.index() == mmo::events::accept::value)
//             return;
//         if (event.index() == mmo::events::handshake::value)
//             return;
//         if (event.index() != mmo::events::upgrade::value)
//             throw result::bad_event_type{};
//         throw result::no_error{};
//     });

//     asio::connect(socket.next_layer(), endpoints);
//     socket.handshake(asio::ssl::stream_base::client);
//     boost::beast::websocket::stream<asio::ssl::stream<asio::ip::tcp::socket>> ws(std::move(socket));

//     ws.handshake("localhost", "/");
//     ASSERT_THROW(step1.get(), result::no_error);
// }

// TEST(network, case_04_auth)
// {
//     MMO_TESTS_PRELUDE({
//         if (event.index() == mmo::events::accept::value)
//             return;
//         if (event.index() == mmo::events::handshake::value)
//             return;
//         if (event.index() == mmo::events::upgrade::value)
//             return;
//         if (event.index() == mmo::events::read::value)
//             return;
//         if (event.index() != mmo::events::auth::value)
//             throw result::bad_event_type{};

//         auto auth_event = std::get<mmo::events::auth>(event);

//         if (std::string(auth_event.nickname) != "nick123")
//             throw result::nickname_dont_match{};
//         else if (std::string(auth_event.password) != "pass123")
//             throw result::password_dont_match{};

//         throw result::no_error{};
//     });

//     asio::connect(socket.next_layer(), endpoints);
//     socket.handshake(asio::ssl::stream_base::client);
//     boost::beast::websocket::stream<asio::ssl::stream<asio::ip::tcp::socket>> ws(std::move(socket));

//     ws.handshake("localhost", "/");

//     mmo::protocol::authentication auth;

//     auth.nickname = "nick123";
//     auth.password = "pass123";

//     std::stringstream ss;
//     {
//         cereal::JSONOutputArchive oarchive(ss);

//         oarchive(auth);
//     }

//     ws.write(asio::buffer(ss.str()));
//     ASSERT_THROW(step1.get(), result::no_error);
// }
