#include "instance.hpp"

#include <stdarg.h>
#include <string>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/detail/chrono.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/system/detail/error_code.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "event.hpp"
#include "session.hpp"
#include "sqlite_database.hpp"

using namespace boost;

#define _INSTANCE_LOG(level, fmt, ...) \
    spdlog::level("[\033[38;2;0;210;120m{:15}\033[39;49m]" fmt, "server", __VA_ARGS__)

namespace mmo {

struct inner {
    boost::asio::io_context                         io_context;
    std::shared_ptr<boost::asio::ssl::context>      ssl_context;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    std::vector<std::shared_ptr<tls_session>>       player_sessions;
    std::function<user_callback_proto>              user_callback;

    inner(std::function<user_callback_proto>&& user_callback)
        : user_callback(std::move(user_callback))
    {}

    void on_accept(game* owner, const boost::system::error_code& ec, boost::asio::ip::tcp::socket socket)
    {
        if (ec)
        {
            _INSTANCE_LOG(warn, "server: accept failed");
            return;
        }
        _INSTANCE_LOG(info, "server: new connection");
        user_callback(events::accept{});

        auto new_session = std::make_shared<tls_session>(std::move(socket), *ssl_context, user_callback);
        new_session->run_async();
        player_sessions.push_back(new_session);

        owner->do_accept();
    }
};

game::game(std::string const& world_name, short port, std::string const& cert_pem, std::string const& key_pem,
           std::function<user_callback_proto>&& user_callback)
    : inner_(reinterpret_cast<void*>(new inner(std::move(user_callback))))
    , port_(port)
    , database_(world_name)
{
    auto inner_data = reinterpret_cast<inner*>(inner_);

    inner_data->acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(inner_data->io_context);
    inner_data->acceptor->open(boost::asio::ip::tcp::v4());
    inner_data->acceptor->set_option(boost::asio::socket_base::reuse_address(true));
    inner_data->acceptor->bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_));
    inner_data->acceptor->listen();

    inner_data->ssl_context = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23_server);
    inner_data->ssl_context->set_options(boost::asio::ssl::context::default_workarounds);

    inner_data->ssl_context->use_certificate_chain(asio::buffer(cert_pem));
    inner_data->ssl_context->use_private_key(asio::buffer(key_pem), boost::asio::ssl::context::pem);
}

game::~game()
{
    auto inner_data = reinterpret_cast<inner*>(inner_);
    if (inner_data)
    {
        delete inner_data;
        inner_ = nullptr;
    }
}

std::jthread
game::launch()
{
    return std::jthread([this](std::stop_token stoken) {
        _INSTANCE_LOG(info, "server: io_context started on port {}", port_);
        do_accept();
        auto                      inner_data = reinterpret_cast<inner*>(inner_);
        boost::asio::steady_timer timer(inner_data->io_context, boost::asio::chrono::milliseconds(250));
        timer.async_wait([this, inner_data, stoken](const boost::system::error_code& ec) {
            _INSTANCE_LOG(info, "Tick!");
            // inner_data->io_context.restart();
            if (stoken.stop_requested())
            {
                _INSTANCE_LOG(info, "server: stop requested");
                return;
            }
            inner_data->acceptor->close();
        });
        inner_data->io_context.run();
        _INSTANCE_LOG(info, "server: io_context stopped");
    });
}

// void
// instance::run()
// {}

void
game::do_accept()
{
    auto inner_data = reinterpret_cast<inner*>(inner_);
    inner_data->acceptor->async_accept(
        std::bind(&inner::on_accept, inner_data, this, std::placeholders::_1, std::placeholders::_2));
    _INSTANCE_LOG(trace, "server: listenning on {}", port_);
}

}   // namespace mmo
