#include <iostream>
#include <thread>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <spdlog/spdlog.h>

#include "../include/hyperblock/utils.hpp"

using namespace boost;

namespace hyper_block {

void
do_session(asio::ip::tcp::socket socket)
{
    try
    {
        beast::flat_buffer buffer;

        beast::http::request<beast::http::string_body> req;
        beast::http::read(socket, buffer, req);

        beast::http::response<beast::http::string_body> res{beast::http::status::ok, req.version()};
        res.set(beast::http::field::server, "Boost Beast HTTP Server");
        res.set(beast::http::field::content_type, "text/plain");
        res.body() = "Hello, World!";
        res.prepare_payload();

        beast::http::write(socket, res);
    } catch (beast::system_error const& se)
    {
        if (se.code() != beast::http::error::end_of_stream)
        {
            spdlog::warn("http service: error: {}", se.code().message());
        }
    } catch (std::exception const& ex)
    {
        spdlog::warn("http service: error: {}", ex.what());
    }
}
void do_accept(asio::ip::tcp::acceptor& acceptor);

void
on_accept(asio::ip::tcp::acceptor& acceptor, system::error_code ec, asio::ip::tcp::socket socket)
{
    if (!ec)
        do_session(std::move(socket));
    do_accept(acceptor);
}

void
do_accept(asio::ip::tcp::acceptor& acceptor)
{
    acceptor.async_accept(std::bind(&on_accept, std::ref(acceptor), std::placeholders::_1, std::placeholders::_2));
}

void
run_validation_service(unsigned short port)
{
    asio::io_context        ioc{static_cast<int>(std::thread::hardware_concurrency())};
    asio::ip::tcp::acceptor acceptor{ioc, {asio::ip::tcp::v4(), static_cast<unsigned short>(port)}};

    acceptor.open(boost::asio::ip::tcp::v4());
    acceptor.set_option(boost::asio::socket_base::reuse_address(true));
    acceptor.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
    acceptor.listen();

    spdlog::warn("HTTP listenning on port", port);

    std::vector<std::thread> threads;

    do_accept(acceptor);

    for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i)
    {
        threads.emplace_back([&] { ioc.run(); });
    }
    for (auto& t : threads)
    {
        t.join();
    }
}

}   // namespace hyper_block
