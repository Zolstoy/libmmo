#pragma once

#include <cstdlib>
#include <memory>

#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/beast/websocket/stream_base.hpp>
#include <boost/system/detail/error_code.hpp>

#include <cereal/archives/json.hpp>
#include <spdlog/spdlog.h>

#include "include/hyperblock/event.hpp"
#include "include/hyperblock/protocol.hpp"


#define _SESSION_LOG(level, fmt, ...) \
    spdlog::level("[\033[38;2;0;210;120m{:15}\033[39;49m]" fmt, client_endpoint_.address().to_string(), __VA_ARGS__)

namespace hyper_block {

template <bool Tls = true>
class session : public std::enable_shared_from_this<session<Tls>>
{
   public:
    using next_layer_type =
        std::conditional_t<Tls, boost::asio::ssl::stream<boost::beast::tcp_stream>, boost::beast::tcp_stream>;
    using websocket_type = boost::beast::websocket::stream<next_layer_type, true>;

   protected:
    boost::asio::ip::tcp::endpoint      client_endpoint_;
    websocket_type                      ws_;
    boost::beast::flat_buffer           buffer_;
    std::function<user_callback_proto> &step_callback_;

   public:
    session(boost::asio::ip::tcp::socket &&socket, std::function<user_callback_proto> &step_callback)
        : client_endpoint_(socket.local_endpoint())
        , ws_(std::move(socket))
        , step_callback_(step_callback)
    {}

    session(boost::asio::ip::tcp::socket &&socket, boost::asio::ssl::context &ctx,
            std::function<user_callback_proto> &step_callback)
        : client_endpoint_(socket.local_endpoint())
        , ws_(std::move(socket), ctx)
        , step_callback_(step_callback)
    {}

   public:
    void run_async()
    {
        if (!Tls)
        {
            _SESSION_LOG(debug, "plain session: initiating websocket upgrade");
            ws_.async_accept(boost::beast::bind_front_handler(&session::on_accept, session<Tls>::shared_from_this()));
        } else
        {
            _SESSION_LOG(debug, "tls session: initiating TLS upgrade");
            ws_.next_layer().async_handshake(
                boost::asio::ssl::stream_base::server,
                boost::beast::bind_front_handler(&session::on_handshake, session<Tls>::shared_from_this()));
        }
    }

   public:
    void on_handshake(boost::beast::error_code ec)
    {
        _SESSION_LOG(debug, "tls session: handshake done");
        step_callback_(events::handshake{});
        if (ec)
        {
            _SESSION_LOG(warn, "tls session: handshake failed");
            return;
        }
        ws_.async_accept(boost::beast::bind_front_handler(&session::on_accept, session<Tls>::shared_from_this()));
    }

    void on_accept(boost::beast::error_code ec)
    {
        _SESSION_LOG(info, "session: websocket accept");
        if (ec)
        {
            _SESSION_LOG(warn, "session: accept error: {}", ec.message());
            return;
        }
        step_callback_(events::upgrade{});
        do_read();
    }

    void do_read()
    {
        ws_.async_read(buffer_, std::bind(&session<Tls>::on_read, session<Tls>::shared_from_this(),
                                          std::placeholders::_1, std::placeholders::_2));
    }

    void on_read(boost::beast::error_code ec, std::size_t)
    {
        if (ec == boost::beast::websocket::error::closed)
            return;

        if (ec)
        {
            _SESSION_LOG(warn, "session: read error: {}", ec.message());
            return;
        }

        std::string message = boost::beast::buffers_to_string(buffer_.data());
        buffer_.consume(buffer_.size());
        _SESSION_LOG(debug, "session: received message: {}", message);

        step_callback_(events::read{message.data()});

        hyper_block::protocol::authentication auth;

        try
        {
            std::stringstream        ss(message);
            cereal::JSONInputArchive iarchive(ss);
            iarchive(auth);
        } catch (std::exception const &ex)
        {
            _SESSION_LOG(warn, "session: could not deserialize: {}", ex.what());
            return;
        }

        _SESSION_LOG(debug, "session: auth: nickname: {}", auth.nickname);
        _SESSION_LOG(debug, "session: auth: password: {}", auth.password);

        step_callback_(events::auth{auth.nickname.data(), auth.password.data()});

        do_read();
    };
};

using tls_session   = session<true>;
using plain_session = session<false>;

}   // namespace hyper_block
