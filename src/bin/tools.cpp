#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>

#include "hyperblock/database.hpp"
#include "hyperblock/utils.hpp"

int
main(int argc, char** argv)
{
    argparse::ArgumentParser program(argv[0], "0.1.0");

    program.add_argument("db_path").help("path to the world sqlite file");

    argparse::ArgumentParser createdb_command("createdb");

    createdb_command.add_description("create the db to the path given");
    createdb_command.add_argument("-f", "--force").help("Erase an existing file").default_value(false);

    argparse::ArgumentParser adduser_command("adduser");
    adduser_command.add_description("add user to be validated to the given db");
    adduser_command.add_argument("nickname").help("the nickname of the user to add").required();

    argparse::ArgumentParser runhttp_command("runhttp");
    runhttp_command.add_description("run the validation server");
    runhttp_command.add_argument("-p", "--port").help("port to run on").default_value(80);

    program.add_subparser(createdb_command);
    program.add_subparser(adduser_command);
    program.add_subparser(runhttp_command);

    try
    {
        program.parse_args(argc, argv);
    } catch (const std::exception& err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    auto db_path = program.get("db_path");

    if (program.is_subcommand_used("createdb"))
    {
        if (createdb_command.is_used("-f"))
            std::filesystem::remove(db_path);

        hyper_block::database db(db_path);
    } else if (program.is_subcommand_used("adduser"))
    {
        hyper_block::database db(db_path);

        auto        nickname = adduser_command.get("nickname");
        std::string password;
        std::cin >> password;
        if (!db.save_user(nickname, password))
            return 1;
    } else if (program.is_subcommand_used("runhttp"))
    {
        auto port = runhttp_command.get<unsigned short>("--port");

        hyper_block::run_validation_service(port);
    } else
    {
        std::cerr << program;
        return 1;
    }
    return 0;
}
