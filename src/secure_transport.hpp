#pragma once

#include <stdarg.h>

#include <chrono>
#include <functional>

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/detail/chrono.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/detail/error_code.hpp>

#include "session.hpp"

#include "boost/asio/ip/tcp.hpp"

using namespace boost;

namespace mmo {

struct secure_transport : public std::enable_shared_from_this<secure_transport> {
    boost::asio::io_context                    io_context;
    std::shared_ptr<boost::asio::ssl::context> ssl_context;
    boost::asio::ip::tcp::acceptor             acceptor;
    std::vector<std::shared_ptr<tls_session>>  player_sessions;
    unsigned short                             port_;
    on_message_func                            on_message_;
    boost::asio::steady_timer                  timer_;

    secure_transport(unsigned short port, std::vector<uint8_t> const& cert_pem, std::vector<uint8_t> const& key_pem,
                     on_message_func on_message_callback)
        : io_context()
        , acceptor(io_context, {asio::ip::tcp::v4(), port})
        , port_(port)
        , on_message_(std::move(on_message_callback))
        , timer_(io_context)
    {
        acceptor.set_option(boost::asio::socket_base::reuse_address(true));
        // acceptor.listen();

        ssl_context = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12_server);

        ssl_context->use_certificate_chain(asio::buffer(cert_pem));
        ssl_context->use_private_key(asio::buffer(key_pem), boost::asio::ssl::context::pem);
    }

    void on_accept(const boost::system::error_code& ec, boost::asio::ip::tcp::socket socket)
    {
        if (ec)
        {
            // std::println("Server: accept error");
            return;
        }

        // std::println("Server: new connection");
        auto new_session = std::make_shared<tls_session>(std::move(socket), *ssl_context, on_message_);
        new_session->run_async();
        player_sessions.push_back(new_session);

        do_accept();
    }

    void timer_handler(std::function<void()> on_message_callback, const boost::system::error_code& error,
                       unsigned short duration_in_ms)
    {
        if (error)
            return;
        on_message_callback();
        timer_.expires_at(boost::asio::steady_timer::clock_type::now() + std::chrono::milliseconds(duration_in_ms));
        timer_.async_wait(std::bind(&secure_transport::timer_handler, shared_from_this(), on_message_callback,
                                    std::placeholders::_1, duration_in_ms));
    }
    void set_timer(unsigned short duration_in_ms, std::function<void()>&& callback)
    {
        timer_.expires_at(boost::asio::steady_timer::clock_type::now() + std::chrono::milliseconds(duration_in_ms));
        timer_.async_wait(std::bind(&secure_transport::timer_handler, shared_from_this(), callback,
                                    std::placeholders::_1, duration_in_ms));
    }

    void start()
    {
        do_accept();
        io_context.run();
    }

    void stop()
    {
        for (auto& session : player_sessions)
            session->close();
        io_context.stop();
        player_sessions.clear();
    }

    void do_accept()
    {
        // std::println("Server: listenning...");
        acceptor.async_accept(
            std::bind(&secure_transport::on_accept, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }
};

}   // namespace mmo
