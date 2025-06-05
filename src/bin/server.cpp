#include <fstream>
#include <print>

#include <hyperblock/error.hpp>
#include <hyperblock/instance.hpp>

#include <argparse/argparse.hpp>

#include "../common/common.hpp"

// using namespace boost;

int
main(int argc, char** argv)
{
    argparse::ArgumentParser program(argv[0], "0.1.0");

    program.add_argument("db_path").help("path to the world sqlite file").required();
    program.add_argument("cert_pem").help("path to the server cert pem file").required();
    program.add_argument("key_pem").help("path to the server key pem file").required();
    program.add_argument("-p", "--port").help("port to use for TLS connection").default_value(443).required();

    try
    {
        program.parse_args(argc, argv);
    } catch (const std::exception& err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    // Use arguments

    auto cert_path = program.get("cert_pem");
    auto key_path  = program.get("key_pem");
    auto port      = program.get<unsigned short>("--port");

    std::ifstream      cert_file(cert_path);
    std::ostringstream os;
    os << cert_file.rdbuf();
    std::string cert = os.str();

    std::ifstream key_file(key_path);
    os.str("");
    os.clear();
    os << key_file.rdbuf();
    std::string key = os.str();

    auto instance =
        std::make_shared<hyper_block::instance>("world.db", port, cert, key, [](hyper_block::event&&) { return true; });

    hyper_block::init_traces();

#if !defined(HYPERBLOCK_WITH_ASIO)
    auto result = instance->run();

    if (!result)
    {
        auto err = result.error();
        switch (static_cast<hyper_block::error_code>(err.index()))
        {
            case hyper_block::error_code::invalid_certificate:
                std::println(stderr, "Invalid certificate: ", std::get<hyper_block::invalid_certificate>(err).message);
                break;
            case hyper_block::error_code::invalid_private_key:
                std::println(stderr, "Invalid certificate: ", std::get<hyper_block::invalid_private_key>(err).message);
                break;
            default:
                std::println(stderr, "Failed to start instance");
        }
        return 1;
    }
#else
    std::println(stderr, "Build the server again without HYPERBLOCK_WITH_ASIO defined");
#endif
}
