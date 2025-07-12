#pragma once

#include <cstdlib>
#include <memory>
#include <print>

#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/beast/websocket/stream_base.hpp>
#include <boost/system/detail/error_code.hpp>

namespace mmo {

template <bool Tls>
class session;

using on_message_proto = void(std::shared_ptr<session<true>>, std::string const &);
using on_message_func  = std::function<on_message_proto>;

template <bool IsTls = true>
class session : public std::enable_shared_from_this<session<IsTls>>
{
   public:
    using next_layer_type =
        std::conditional_t<IsTls, boost::asio::ssl::stream<boost::beast::tcp_stream>, boost::beast::tcp_stream>;
    using websocket_type = boost::beast::websocket::stream<next_layer_type, true>;

   protected:
    boost::asio::ip::tcp::endpoint client_endpoint_;
    websocket_type                 ws_;
    boost::beast::flat_buffer      buffer_;
    on_message_func               &on_message_;

   public:
    session(boost::asio::ip::tcp::socket &&socket, on_message_func &callback)
        : client_endpoint_(socket.local_endpoint())
        , ws_(std::move(socket))
        , on_message_(callback)
    {}

    ~session()
    {
        ws_.close(boost::beast::websocket::close_code::normal);
    }

    session(boost::asio::ip::tcp::socket &&socket, boost::asio::ssl::context &ctx, on_message_func &callback)
        : client_endpoint_(socket.local_endpoint())
        , ws_(std::move(socket), ctx)
        , on_message_(callback)
    {}

   public:
    void close()
    {
        boost::beast::get_lowest_layer(ws_).close();
    }

    void on_message(on_message_func &&callback)
    {
        on_message_ = callback;
    }

    void run_async()
    {
        if (!IsTls)
        {
            ws_.async_accept(boost::beast::bind_front_handler(&session::on_accept, session<IsTls>::shared_from_this()));
        } else
        {
            // std::println("Session: waiting for handshake...");
            ws_.next_layer().async_handshake(
                boost::asio::ssl::stream_base::server,
                boost::beast::bind_front_handler(&session::on_handshake, session<IsTls>::shared_from_this()));
        }
    }

   public:
    void on_handshake(boost::beast::error_code ec)
    {
        // std::println("On handshake");
        if (ec)
            return;
        ws_.async_accept(boost::beast::bind_front_handler(&session::on_accept, session<IsTls>::shared_from_this()));
    }

    void on_accept(boost::beast::error_code ec)
    {
        if (ec)
            return;
        do_read();
    }

    void do_read()
    {
        ws_.async_read(buffer_, std::bind(&session<IsTls>::on_read, session<IsTls>::shared_from_this(),
                                          std::placeholders::_1, std::placeholders::_2));
    }

    void on_read(boost::beast::error_code ec, std::size_t)
    {
        if (ec)
            return;

        std::string message = boost::beast::buffers_to_string(buffer_.data());
        buffer_.consume(buffer_.size());

        on_message_(session<IsTls>::shared_from_this(), message);

        do_read();
    };
};

using tls_session   = session<true>;
using plain_session = session<false>;

}   // namespace mmo
