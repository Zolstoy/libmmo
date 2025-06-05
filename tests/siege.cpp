#include <boost/asio/io_context.hpp>

#include <mmo/instance.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

using namespace boost;

TEST(siege, scenario_01_normal)
{
    ASSERT_NO_THROW(mmo::instance(get_random_instance_path(), 0, CERT, KEY, [](mmo::event &&) { return true; }));
}
