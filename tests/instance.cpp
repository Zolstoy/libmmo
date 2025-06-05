#include <print>

#include <mmo/event.hpp>
#include <mmo/instance.hpp>
#include <mmo/protocol.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(instance, case_01_construct_nominal)
{
    ASSERT_NO_THROW(mmo::instance(get_random_instance_path(), 0, CERT, KEY, [](mmo::event &&) { return true; }));
}

TEST(instance, case_02_construct_fault)
{
    {
        mmo::instance instance(get_random_instance_path(), 2456, "", KEY, [](mmo::event &&) { return true; });
        ASSERT_FALSE(instance.run());
    }
    {
        mmo::instance instance(get_random_instance_path(), 2456, CERT, "", [](mmo::event &&) { return true; });
        ASSERT_FALSE(instance.run());
    }
    {
        mmo::instance instance(get_random_instance_path(), 2456, "", "", [](mmo::event &&) { return true; });
        ASSERT_FALSE(instance.run());
    }
}

TEST(instance, run_async_nominal)
{
    mmo::instance instance(get_random_instance_path(), 2456, CERT, KEY, [](mmo::event &&) { return true; });
    auto          result = instance.run();
    if (!result)
    {
        std::println("==>{}", result.error().index());
    }
    ASSERT_TRUE(result);
    ASSERT_EQ(mmo::error_code::instance_already_running, static_cast<mmo::error_code>(instance.run().error().index()));
}
