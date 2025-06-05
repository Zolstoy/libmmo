#include <expected>

#include <mmo/database.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(database, case_01_creation)
{
    ASSERT_NO_THROW(mmo::database db(get_random_instance_path()));
}

TEST(database, case_02_saveuser)
{
    mmo::database db(get_random_instance_path());

    std::expected<std::tuple<>, mmo::error> result;

    ASSERT_NO_THROW(result = db.save_user("test123", "pass123"));
    ASSERT_TRUE(result);
}

TEST(database, case_03_loadplayer)
{
    mmo::database db(get_random_instance_path());

    db.set_init_position(24, 42);

    auto result1 = db.save_user("test123", "pass123");

    auto result2 = db.load_user_player("test123", "pass123");

    ASSERT_TRUE(result2);

    mmo::player player = result2.value();

    ASSERT_EQ(24, player.get_pos_x());
    ASSERT_EQ(42, player.get_pos_y());
}
