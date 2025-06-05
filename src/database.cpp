#include "include/hyperblock/database.hpp"

#include <exception>
#include <expected>
#include <mutex>
#include <stdexcept>
#include <string>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Exception.h>
#include <SQLiteCpp/Statement.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <spdlog/spdlog.h>

#include "include/hyperblock/error.hpp"
#include "include/hyperblock/player.hpp"
#include "include/hyperblock/utils.hpp"

namespace hyper_block {

struct inner {
    SQLite::Database sql_db;
    std::mutex       mutex;

    template <class... T>
    inner(T... args)
        : sql_db(args...)
    {}
};

database::database(std::string const& world_name)
    : init_pos_x_(0)
    , init_pos_y_(0)
{
    try
    {
        inner_ = reinterpret_cast<void*>(new inner(world_name, SQLite::OPEN_READWRITE));
    } catch (...)
    {
        inner_ = reinterpret_cast<void*>(new inner(world_name, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE));
        try
        {
            auto inner_data = reinterpret_cast<inner*>(inner_);
            SQLite::Statement(inner_data->sql_db,
                              "CREATE TABLE player ("
                              "id INTEGER PRIMARY KEY,"
                              "pos_x REAL DEFAULT 0,"
                              "pos_y REAL DEFAULT 0"
                              ")")
                .exec();

            SQLite::Statement(inner_data->sql_db,
                              "CREATE TABLE user ("
                              "id INTEGER PRIMARY KEY,"
                              "nickname TEXT,"
                              "pass_hash TEXT,"
                              "salt TEXT,"
                              "player_id INTEGER,"
                              "FOREIGN KEY(player_id) REFERENCES player(id)"
                              ")")
                .exec();

        } catch (SQLite::Exception const& ex)
        {
            spdlog::error("Could not init database: {}", ex.what());
            std::filesystem::remove(world_name);
            throw std::runtime_error("Could not init database");
        }
    }
}

std::expected<std::tuple<>, error>
database::save_user(std::string const& nickname, std::string const& password)
{
    auto              inner_data = reinterpret_cast<inner*>(inner_);
    std::lock_guard   guard(inner_data->mutex);
    auto              salt      = uuid();
    auto              pass_hash = sha256(nickname + password + salt);
    SQLite::Statement insert_player_query(inner_data->sql_db,
                                          "INSERT INTO player (pos_x, pos_y) VALUES"
                                          "(?, ?)");
    insert_player_query.bind(1, init_pos_x_);
    insert_player_query.bind(2, init_pos_y_);

    if (insert_player_query.exec() != 1)
        return std::unexpected(error_not_implemented{});

    unsigned int player_id = inner_data->sql_db.getLastInsertRowid();

    SQLite::Statement insert_user_query(inner_data->sql_db,
                                        "INSERT INTO user (nickname, pass_hash, salt, player_id) VALUES"
                                        "(?, ?, ?, ?)");

    insert_user_query.bind(1, nickname);
    insert_user_query.bind(2, pass_hash);
    insert_user_query.bind(3, salt);
    insert_user_query.bind(4, player_id);

    if (insert_user_query.exec() != 1)
        return std::unexpected(error_not_implemented{});
    return std::tuple<>();
}

std::expected<player, error>
database::load_user_player(std::string const& nickname, std::string const& password)
{
    auto            inner_data = reinterpret_cast<inner*>(inner_);
    std::lock_guard guard(inner_data->mutex);
    try
    {
        SQLite::Statement load_salt_query(inner_data->sql_db,
                                          "SELECT salt"
                                          " FROM user"
                                          " WHERE nickname = ?");
        load_salt_query.bind(1, nickname);
        load_salt_query.executeStep();

        std::string salt = load_salt_query.getColumn(0);

        auto pass_hash = sha256(nickname + password + salt);

        SQLite::Statement load_player_query(inner_data->sql_db,
                                            "SELECT pos_x, pos_y"
                                            " FROM player"
                                            " INNER JOIN user on player.id = user.player_id"
                                            " WHERE user.pass_hash = ?");
        load_player_query.bind(1, pass_hash);
        load_player_query.executeStep();
        double pos_x = load_player_query.getColumn(0);
        double pos_y = load_player_query.getColumn(1);
        return player(pos_x, pos_y);

    } catch (std::exception const& ex)
    {
        return std::unexpected(error_not_implemented{});
    }
}

void
database::set_init_position(float pos_x, float pos_y)
{
    auto            inner_data = reinterpret_cast<inner*>(inner_);
    std::lock_guard guard(inner_data->mutex);
    init_pos_x_ = pos_x;
    init_pos_y_ = pos_y;
}

}   // namespace hyper_block
