#include <cstddef>

#include <mmo/mmo.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(test_01_start, case_01_call_nominal)
{
    auto converted_cert = reinterpret_cast<uint8_t const *>(CERT.data());
    auto converted_key  = reinterpret_cast<uint8_t const *>(KEY.data());
    ASSERT_NO_THROW(mmo::start(
        [](float, std::map<size_t, std::shared_ptr<mmo::player>> const &) -> mmo::order { return mmo::order::stop; },
        std::vector(converted_cert, converted_cert + CERT.size()),
        std::vector(converted_key, converted_key + KEY.size()), 250, 8685));
}

TEST(test_01_start, case_02_call_failure_bad_cert)
{
    auto converted_cert = reinterpret_cast<uint8_t const *>(CERT.data());
    auto converted_key  = reinterpret_cast<uint8_t const *>(KEY.data());
    ASSERT_ANY_THROW(mmo::start(
        [](float, std::map<size_t, std::shared_ptr<mmo::player>> const &) -> mmo::order { return mmo::order::stop; },
        std::vector(converted_cert, converted_cert), std::vector(converted_key, converted_key + KEY.size()), 250,
        8685));
}

TEST(test_01_start, case_02_call_failure_bad_key)
{
    auto converted_cert = reinterpret_cast<uint8_t const *>(CERT.data());
    auto converted_key  = reinterpret_cast<uint8_t const *>(KEY.data());
    ASSERT_ANY_THROW(mmo::start(
        [](float, std::map<size_t, std::shared_ptr<mmo::player>> const &) -> mmo::order { return mmo::order::stop; },
        std::vector(converted_cert, converted_cert + CERT.size()), std::vector(converted_key, converted_key), 250,
        8685));
}

int
main(int argc, char **argv)
{
    // init_traces();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
