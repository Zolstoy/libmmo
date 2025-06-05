#include <boost/asio/io_context.hpp>

#include <hyperblock/instance.hpp>

#include <gtest/gtest.h>

#include "../common/common.hpp"

using namespace boost;

TEST(siege, scenario_01_normal)
{
    asio::io_context ioc;
    ASSERT_NO_THROW(hyper_block::instance(ioc, get_random_instance_path(), 0, CERT, KEY,
                                          [](hyper_block::event &&) { return true; }));
}
