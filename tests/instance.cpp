#include <mmo/event.hpp>
#include <mmo/instance.hpp>
#include <mmo/protocol.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(instance, case_01_construct_nominal)
{
    ASSERT_NO_THROW(mmo::instance(get_random_instance_path(), 2456, CERT, KEY, [](mmo::event &&) {}));
}

TEST(instance, case_02_construct_fault)
{
    {
        ASSERT_ANY_THROW(mmo::instance instance(get_random_instance_path(), 2456, "", KEY, [](mmo::event &&) {}));
    }
    {
        ASSERT_ANY_THROW(mmo::instance instance(get_random_instance_path(), 2456, CERT, "", [](mmo::event &&) {}));
    }
    {
        ASSERT_ANY_THROW(mmo::instance instance(get_random_instance_path(), 2456, "", "", [](mmo::event &&) {}));
    }
}

TEST(instance, run_async_nominal)
{
    mmo::instance instance(get_random_instance_path(), 2456, CERT, KEY, [](mmo::event &&) { return true; });
    auto          thrd = instance.launch();
    ASSERT_TRUE(thrd.joinable());
    thrd.request_stop();
    thrd.join();
}
