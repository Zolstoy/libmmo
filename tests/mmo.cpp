#include <cstddef>
#include <future>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <mmo/mmo.hpp>

#include <gtest/gtest.h>

#include "boost/asio/io_context.hpp"
#include "boost/asio/ssl/context.hpp"
#include "boost/asio/ssl/stream.hpp"
#include "boost/asio/ssl/stream_base.hpp"
#include "boost/system/detail/error_code.hpp"

using namespace boost;
using namespace std::chrono_literals;

class test_base : public ::testing::Test
{
   private:
    static constexpr std::string_view CA_CERT =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDGzCCAgOgAwIBAgIUVlpyalwiQIyyrcHPGXGm+1fEPMIwDQYJKoZIhvcNAQEL"
        "BQAwHTELMAkGA1UEBhMCRkkxDjAMBgNVBAMMBXZhaGlkMB4XDTI0MTIwMTIwMjEy"
        "NVoXDTI5MTEzMDIwMjEyNVowHTELMAkGA1UEBhMCRkkxDjAMBgNVBAMMBXZhaGlk"
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAk/W74DzJBDOw5OW+EXSN"
        "gMAfmgZnRc6sP698IcrsBFs78VqB0donQqltnD43Ohxe+iHDGdHI1H4I3dY3OgCY"
        "HSIibJEkCfO4z1A3NtsNI8y2+AO3QKhMm9XK4TwMW9aFCnaocB+SbIbfmSiW5tfU"
        "KXfVp8ya0ieAO5zTEkhXX6ZGqr1gFtyM7wx3pjUuzffMnFQPrIZoY9JxBe3qnPED"
        "mkjC5qTxKytAfb6PpYYSl+jhnykfsMyR9IrypwUIG+IXImPd8y/6+m6JN06fwQWV"
        "p49hu3XvvtGOEU23tEbgDQR5t0AjKMlHmT2Y0WG6GsAnDALnNBkGq7ZNrk17Mw91"
        "VQIDAQABo1MwUTAdBgNVHQ4EFgQUOl11VxPYjLse2i7pNIXEc+Nn/iUwHwYDVR0j"
        "BBgwFoAUOl11VxPYjLse2i7pNIXEc+Nn/iUwDwYDVR0TAQH/BAUwAwEB/zANBgkq"
        "hkiG9w0BAQsFAAOCAQEAH0QgIq509cxFwSxqZRpbLBuHbdUq+xFB42N0ttDNJZzi"
        "T01OWsPYtim8/WXlYC5PHv1FZthY9/7Ci2tEicm6X01CNnvNgeZx8bBGpOq0rqkY"
        "+9xRPSQXVoIbApg3KHDeUq6Fe9leASFohEbXk7gbi9c1yuT4Z+O19KmY8/rtvR1N"
        "U9c0sNvcDC5Q4bVai6KAhLxzLCBaYSqY4ku881K3pBSNVEy5gBVj466DOFNLPNg6"
        "Oha9NBAsvMsXonrrYDYtwk92p3L9O55b/YKG0MYW4qCB27SZnYZwDea9+h/MLvFV"
        "lBjhUjWT859gkyO6pYSTfndSpnWAdtQK9zsTYociBQ==\n"
        "-----END CERTIFICATE-----";

    static constexpr std::string_view SERVER_CERT =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDMTCCAhmgAwIBAgIUPW2I5vQZWOxWMHqP1Pu73GfKvhUwDQYJKoZIhvcNAQEL"
        "BQAwHTELMAkGA1UEBhMCRkkxDjAMBgNVBAMMBXZhaGlkMB4XDTI0MTIwMTIwMzAw"
        "NFoXDTI1MTIwMTIwMzAwNFowHTELMAkGA1UEBhMCRkkxDjAMBgNVBAMMBXZhaGlk"
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtcLRSlxRbbOT4m1vKeWm"
        "HRxSpr6YdHT4TlJYcQnvNg7NQBoSQFLTY/c9vDwnwpC3nDc+I3VauZYb44Iocnht"
        "BK7AQPyscjM6dwVu0mxFIgc0i2t5+yrNs8n5jWzHsMu7ZgMc9RmRBzgadw/9VHcH"
        "RyFJt1wYIJI48PjNW/IfzeGYCNEjTdWYifBdZKt4gOrpcEvHzlsjebcVdXTrS8sI"
        "82zLKCGfy07JqDxHhMb4uIb/J/SKNkng2Dpr9Ythxfn5dD4BKuaKrEnxjLxBKX3J"
        "SUa5+bs3lP/LH5nz/cogBV6t6BIoJ7p//jgjSalCkXvGnKG/+asid1JJ0z5ZuM/R"
        "KwIDAQABo2kwZzAfBgNVHSMEGDAWgBQ6XXVXE9iMux7aLuk0hcRz42f+JTAJBgNV"
        "HRMEAjAAMBoGA1UdEQQTMBGCCWxvY2FsaG9zdIcEfwAAATAdBgNVHQ4EFgQU6Yab"
        "dvv0NBb/mYRdbOzN3T+gUcYwDQYJKoZIhvcNAQELBQADggEBAFLoifH57rdSzLV/"
        "ZuOGEKvn/KgAcM+p+Sj7vujwe+vntMMBSjChm98YsOPR26j0aweKfHlnrbPuerk1"
        "dvU34pe0v0TDzLIpJuIkfZe5MMx3WjvhwTPOWlAqxaMMxAD+95I6KChP4lV9xqLv"
        "iPgSDSODElS/qKb3kU4sA4m2CxmI6yCWW2tYsjoTkqrBmhjKql6UnBBrkb5K6tXm"
        "jcg0sq+u24j0Hzq9slk3Uxk3viqdN1X6p1sPCeAdO7Q2y6NBB8rTYu6klUQQRWL8"
        "NH4has89I4jp2ufcy1zY4ckN3uSZffG8S+v3jv/c9dmZoV7OO1CYnwvzgo01k9GD"
        "Vqi4i7M=\n"
        "-----END CERTIFICATE-----";

    static constexpr std::string_view SERVER_KEY =
        "-----BEGIN PRIVATE KEY-----\n"
        "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC1wtFKXFFts5Pi"
        "bW8p5aYdHFKmvph0dPhOUlhxCe82Ds1AGhJAUtNj9z28PCfCkLecNz4jdVq5lhvj"
        "gihyeG0ErsBA/KxyMzp3BW7SbEUiBzSLa3n7Ks2zyfmNbMewy7tmAxz1GZEHOBp3"
        "D/1UdwdHIUm3XBggkjjw+M1b8h/N4ZgI0SNN1ZiJ8F1kq3iA6ulwS8fOWyN5txV1"
        "dOtLywjzbMsoIZ/LTsmoPEeExvi4hv8n9Io2SeDYOmv1i2HF+fl0PgEq5oqsSfGM"
        "vEEpfclJRrn5uzeU/8sfmfP9yiAFXq3oEignun/+OCNJqUKRe8acob/5qyJ3UknT"
        "Plm4z9ErAgMBAAECggEAHOKT/hxDuIpUUySPCPp89p1cqTEa6073cwL1GSm6AT5C"
        "8g/ynJRNEdLl1bc9nlb/Ru0ki+AHhfzL+9DgeqiWsqrO1MUS5qcrgGS1ou0f43N/"
        "rzRqUzcPL6ZGaWpDJd6KroCKJo1kleAdnJRG7xhnaK9qlqAlGXADapAvmpAU69PM"
        "MwpW9S96QvVHfPP7LXO/nvNzqLnrNysprHkSH6iV4ao37LEqzgUF0tABTk0Q67UJ"
        "O4XSToMAJ8GOBjYSKVK3PJm3saqTobff9Oz2HgUWUyr92kSESPhfNEVlMskmgvE3"
        "CcajxOxudxg94AAU7Es1UE5bMtY2e/Cs1088yzC3SQKBgQDvtYHI+4Kcur2ply0p"
        "QIBSSspJZ7fGT9/waK0EFlAyQ/qAaFH0Ilb6U2/L52TSR0EbSImQN7VxkUrosHym"
        "HahB6yHXkI2G8nDcmSdNjyiiC00+LWyKCtixE+bRCAuReZmypSk1Fz8GwYb3gaBR"
        "YcsWGsMeomFpL6q6yIgo43r8xQKBgQDCHR9fciT7zHTWAyPNlPLVzuJlvi164OC8"
        "GkHHxx+CybIDZVrUdUfYk80kxC+bvlUIaMs2D0MVUg2Hv8IbtMjEs+FV4vM/Df9J"
        "e9SWhOTWz25Jc7ZRYKVKc848l6TQd5JMU4JjeqmmVAza27l6Iu4TQb+r9GrZgBxX"
        "6NBj8vZVLwKBgFsW1iLRsGhubfQsBnVOlXSwBv6t8x/g6nAo1tZexErVmjOBcOMc"
        "yYCGhE0vuRhPC2aaweuTv9dQJu8VYcieLHogJ9QKkj1dk5XAfTbz17T8JnYiPMSY"
        "Ko/fyC5WqE63rrg8GtSZ6NFgaTFUiN9kEhBsSwkxG2MlQfOIkHU5PFshAoGBAL6c"
        "4GjWapDERdq9/JNs90STQmgMZxap6qVr1zp5Q20n6GFDTv0gKav3/1NiPyndrhxy"
        "41GzjPlLuLObzt1sGlZmGRlAogJCGXSsX6Zq21hBGxiPwvGISOeiblu7wYFgWU4Q"
        "FxLeqecF1BW5/Bl+YXCReMk/Wwk3rx14JeJv/ArLAoGAPwBXLX1HwQeHoFn4ImZV"
        "r0fUKkD4LzaAJ4gbEqzAQ8AD8vmqq+CBpu1YCLO6SFqHsFj1RUfk1ScVVD9tlL7E"
        "CI5ivNoxDpThvZhP6v42T7JQKK49YaGySE/k3y0wztfsk8qn6dAI6pwFMgtfsFFo"
        "RZb6vjD6zPWZElSkrwGczDM=\n"
        "-----END PRIVATE KEY-----";

   public:
    static constexpr unsigned short DEFAULT_PORT = 8685;

   public:
    std::vector<uint8_t> const ca_cert;
    std::vector<uint8_t> const server_cert;
    std::vector<uint8_t> const server_key;

   private:
    size_t           cnt_;
    asio::io_context ioc_;

   public:
    test_base()
        : ca_cert(reinterpret_cast<uint8_t const *>(CA_CERT.data()),
                  reinterpret_cast<uint8_t const *>(CA_CERT.data()) + CA_CERT.size())
        , server_cert(reinterpret_cast<uint8_t const *>(SERVER_CERT.data()),
                      reinterpret_cast<uint8_t const *>(SERVER_CERT.data()) + SERVER_CERT.size())
        , server_key(reinterpret_cast<uint8_t const *>(SERVER_KEY.data()),
                     reinterpret_cast<uint8_t const *>(SERVER_KEY.data()) + SERVER_KEY.size())
        , cnt_(0)
    {}
    mmo::game_cycle get_cycle(size_t n_cycles = 1)
    {
        return [cnt = std::ref(cnt_), n = n_cycles](
                   float, std::map<size_t, std::shared_ptr<mmo::player>> const &) -> mmo::order {
            ++cnt;
            if (cnt == n)
                return mmo::order::stop;
            return mmo::order::keep_going;
        };
    }

    size_t get_cnt()
    {
        return cnt_;
    }

    asio::io_context &get_ioc()
    {
        return ioc_;
    }
};

using test_01_start   = test_base;
using test_02_cycle   = test_base;
using test_03_network = test_base;

TEST_F(test_01_start, case_01_call_nominal)
{
    ASSERT_NO_THROW(mmo::start(get_cycle(), server_cert, server_key, 1, ::rand()));
    ASSERT_EQ(1, get_cnt());
}

TEST_F(test_01_start, case_02_call_failure_bad_cert)
{
    ASSERT_ANY_THROW(
        mmo::start(get_cycle(), std::vector(server_cert.cbegin(), server_cert.cbegin()), server_key, 1, ::rand()));
    ASSERT_EQ(0, get_cnt());
}

TEST_F(test_01_start, case_03_call_failure_bad_key)
{
    ASSERT_ANY_THROW(
        mmo::start(get_cycle(), server_cert, std::vector(server_key.cbegin(), server_key.cbegin()), 1, ::rand()));
    ASSERT_EQ(0, get_cnt());
}

TEST_F(test_01_start, case_04_call_failure_tick_zero)
{
    ASSERT_ANY_THROW(mmo::start(get_cycle(), server_cert, server_key, 0, ::rand()));
    ASSERT_EQ(0, get_cnt());
}

TEST_F(test_02_cycle, case_01_two_ticks)
{
    auto n_ticks = 2;
    auto fut     = std::async([&] { mmo::start(get_cycle(n_ticks), server_cert, server_key, 1, ::rand()); });

    ASSERT_NO_THROW(fut.get());
    ASSERT_EQ(n_ticks, get_cnt());
}

TEST_F(test_02_cycle, case_02_five_hundred_ticks)
{
    auto n_ticks = 500;
    auto fut     = std::async([&] { mmo::start(get_cycle(n_ticks), server_cert, server_key, 1, ::rand()); });

    ASSERT_NO_THROW(fut.get());
    ASSERT_EQ(n_ticks, get_cnt());
}

TEST_F(test_02_cycle, case_03_hundred_ticks_ten_millis)
{
    auto n_ticks = 100;
    auto fut     = std::async([&] { mmo::start(get_cycle(n_ticks), server_cert, server_key, 10, ::rand()); });

    ASSERT_NO_THROW(fut.get());
    ASSERT_EQ(n_ticks, get_cnt());
}

TEST_F(test_02_cycle, case_04_ten_ticks_hundred_millis)
{
    auto n_ticks = 10;
    auto fut     = std::async([&] { mmo::start(get_cycle(n_ticks), server_cert, server_key, 100, ::rand()); });

    ASSERT_NO_THROW(fut.get());
    ASSERT_EQ(n_ticks, get_cnt());
}

TEST_F(test_02_cycle, case_05_one_tick_thousand_millis)
{
    auto n_ticks = 1;
    auto fut     = std::async([&] { mmo::start(get_cycle(n_ticks), server_cert, server_key, 1000, ::rand()); });

    ASSERT_NO_THROW(fut.get());
    ASSERT_EQ(n_ticks, get_cnt());
}

TEST_F(test_03_network, case_01_one_connection)
{
    unsigned short port = ::rand();
    auto fut = std::async(std::launch::async, [&] { mmo::start(get_cycle(3), server_cert, server_key, 1000, port); });
    std::this_thread::sleep_for(500ms);

    asio::ip::tcp::socket          socket(get_ioc());
    boost::asio::ip::tcp::resolver resolver(get_ioc());

    auto const results = resolver.resolve("localhost", std::to_string(port));
    boost::asio::connect(socket, results);
    ASSERT_NO_THROW(fut.get());
}

TEST_F(test_03_network, case_02_one_handshake)
{
    unsigned short port = ::rand();
    auto fut = std::async(std::launch::async, [&] { mmo::start(get_cycle(3), server_cert, server_key, 1000, port); });
    std::this_thread::sleep_for(500ms);

    asio::ip::tcp::socket          socket(get_ioc());
    boost::asio::ip::tcp::resolver resolver(get_ioc());

    auto const results = resolver.resolve("localhost", std::to_string(port));

    boost::asio::connect(socket, results);

    asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12_client);
    ssl_ctx.add_certificate_authority(asio::buffer(ca_cert));
    ssl_ctx.set_verify_mode(asio::ssl::verify_peer);
    asio::ssl::stream<decltype(socket)> secure_socket(std::move(socket), ssl_ctx);

    if (!SSL_set_tlsext_host_name(secure_socket.native_handle(), "localhost"))
    {
        boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
        throw boost::system::system_error{ec};
    }

    secure_socket.handshake(asio::ssl::stream_base::client);

    ASSERT_NO_THROW(fut.get());
}
