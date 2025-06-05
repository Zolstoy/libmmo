#include <expected>
#include <print>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/beast.hpp>

#include <hyperblock/instance.hpp>
#include <hyperblock/protocol.hpp>

#include <cereal/archives/json.hpp>
#include <fmt/color.h>
#include <gtest/gtest.h>
#include <spdlog/common.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "hyperblock/event.hpp"

#include "../common/common.hpp"

using namespace boost;

TEST(instance, case_01_construct_nominal)
{
    asio::io_context ioc;

    ASSERT_NO_THROW(hyper_block::instance(ioc, get_random_instance_path(), 0, CERT, KEY,
                                          [](hyper_block::event &&) { return true; }));
}

TEST(instance, case_02_construct_fault)
{
    asio::io_context ioc;
    {
        hyper_block::instance instance(ioc, get_random_instance_path(), 2456, "", KEY,
                                       [](hyper_block::event &&) { return true; });
        ASSERT_FALSE(instance.run_async());
    }
    {
        hyper_block::instance instance(ioc, get_random_instance_path(), 2456, CERT, "",
                                       [](hyper_block::event &&) { return true; });
        ASSERT_FALSE(instance.run_async());
    }
    {
        hyper_block::instance instance(ioc, get_random_instance_path(), 2456, "", "",
                                       [](hyper_block::event &&) { return true; });
        ASSERT_FALSE(instance.run_async());
    }
}

TEST(instance, run_async_nominal)
{
    asio::io_context      ioc;
    hyper_block::instance instance(ioc, get_random_instance_path(), 2456, CERT, KEY,
                                   [](hyper_block::event &&) { return true; });
    auto                  result = instance.run_async();
    if (!result)
    {
        std::println("==>{}", result.error().index());
    }
    ASSERT_TRUE(result);
    ASSERT_EQ(hyper_block::error_code::instance_already_running,
              static_cast<hyper_block::error_code>(instance.run_async().error().index()));
}

int
main(int argc, char **argv)
{
    hyper_block::init_traces();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
