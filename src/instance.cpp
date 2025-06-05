#include "include/hyperblock/instance.hpp"

#include <expected>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/system/detail/error_code.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "include/hyperblock/database.hpp"
#include "include/hyperblock/error.hpp"
#include "include/hyperblock/event.hpp"

#include "session.hpp"


using namespace boost;

#define _INSTANCE_LOG(level, fmt, ...) \
    spdlog::level("[\033[38;2;0;210;120m{:15}\033[39;49m]" fmt, "server", __VA_ARGS__)

namespace hyper_block {

struct inner {
#ifdef HYPERBLOCK_WITH_ASIO
    boost::asio::io_context& io_context;
#else
    boost::asio::io_context io_context;
#endif
    std::shared_ptr<boost::asio::ssl::context>      ssl_context;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    std::vector<std::shared_ptr<tls_session>>       player_sessions;
    std::vector<std::shared_ptr<plain_session>>     bot_sessions;
    std::string                                     cert_pem;
    std::string                                     key_pem;
    std::function<user_callback_proto>              user_callback;

#ifdef HYPERBLOCK_WITH_ASIO
    inner(boost::asio::io_context& io_context, std::string const& cert_pem, std::string const& key_pem,
          std::function<user_callback_proto>&& user_callback)
        : io_context(io_context)
        , cert_pem(cert_pem)
        , key_pem(key_pem)
        , user_callback(std::move(user_callback))
    {}
#else
    inner(std::string const& cert_pem, std::string const& key_pem, std::function<user_callback_proto>&& user_callback)
        : cert_pem(cert_pem)
        , key_pem(key_pem)
        , user_callback(std::move(user_callback))
    {}
#endif

    void on_accept(instance* owner, const boost::system::error_code& ec, boost::asio::ip::tcp::socket socket)
    {
        user_callback(events::accept{});
        if (ec)
        {
            _INSTANCE_LOG(warn, "server: accept failed");
            owner->is_running_ = false;
            return;
        }
        _INSTANCE_LOG(info, "server: new connection");

        auto new_session = std::make_shared<tls_session>(std::move(socket), *ssl_context, user_callback);
        new_session->run_async();
        player_sessions.push_back(new_session);

        owner->do_accept();
    }
};

#ifdef HYPERBLOCK_WITH_ASIO
instance::instance(asio::io_context& io_context, std::string const& world_name, short port, std::string const& cert_pem,
                   std::string const& key_pem, std::function<user_callback_proto>&& step_callback) noexcept
    : inner_(reinterpret_cast<void*>(new inner(io_context, cert_pem, key_pem, std::move(step_callback))))
    , port_(port)
    , is_running_(false)
    , database_(world_name)
{}
#else
instance::instance(std::string const& world_name, short port, std::string const& cert_pem, std::string const& key_pem,
                   std::function<user_callback_proto>&& step_callback) noexcept
    : inner_(reinterpret_cast<void*>(new inner(cert_pem, key_pem, std::move(step_callback))))
    , port_(port)
    , is_running_(false)
    , database_(world_name)
{}
#endif

HYPERBLOCK_API std::expected<short, error>
               instance::run_async() noexcept
{
    auto inner_data = reinterpret_cast<inner*>(inner_);
    if (is_running_)
        return std::unexpected(instance_already_running{});

    inner_data->acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(inner_data->io_context);
    try
    {
        inner_data->acceptor->open(boost::asio::ip::tcp::v4());
        inner_data->acceptor->set_option(boost::asio::socket_base::reuse_address(true));
        inner_data->acceptor->bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_));
        inner_data->acceptor->listen();

        inner_data->ssl_context = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
        inner_data->ssl_context->set_options(boost::asio::ssl::context::default_workarounds);
    } catch (...)
    {
        return std::unexpected(acceptor_failed{});
    }

    boost::system::error_code ec;
    if (inner_data->ssl_context->use_certificate_chain(asio::buffer(inner_data->cert_pem), ec))
        return std::unexpected(invalid_certificate{ec.message()});
    if (inner_data->ssl_context->use_private_key(asio::buffer(inner_data->key_pem), boost::asio::ssl::context::pem, ec))
        return std::unexpected(invalid_private_key{ec.message()});
    try
    {
        do_accept();
    } catch (...)
    {
        return std::unexpected(acceptor_failed{});
    }
    is_running_ = true;
    return 0;
}

#if !defined(HYPERBLOCK_WITH_ASIO)
std::expected<std::tuple<>, error>
instance::run() noexcept
{
    std::vector<std::thread> threads;
    asio::io_context         ioc;

    auto result = run_async();

    if (!result)
        return std::unexpected(result.error());
    for (int i = 0; i < std::thread::hardware_concurrency(); ++i)
    {
        threads.emplace_back([&] {
            try
            {
                ioc.run();
            } catch (std::exception& e)
            {
                std::println(std::cerr, "{}", e.what());
            }
        });
    }

    for (auto& t : threads)
        t.join();
    return std::tuple<>();
}
#endif

void
instance::do_accept()
{
    auto inner_data = reinterpret_cast<inner*>(inner_);
    inner_data->acceptor->async_accept(
        std::bind(&inner::on_accept, inner_data, this, std::placeholders::_1, std::placeholders::_2));
    _INSTANCE_LOG(trace, "server: listenning on {}", port_);
}

}   // namespace hyper_block
