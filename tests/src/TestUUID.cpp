#include <gtest/gtest.h>

#include <Scheduler/Common/Console.h>
#include <Scheduler/Lib/UUID.h>
#include <iostream>
#include <vector>

using namespace Scheduler;
using Scheduler::Lib::UUID;

TEST(UUIDInit, Initialize)
{
    UUID uuidA(true), uuidB(true);
    Console(std::cout) << "UUID: " << uuidA << '\n';
    Console(std::cout) << "UUID: " << uuidB << '\n';
    ASSERT_NE(uuidA, uuidB);
}

TEST(UUIDInit, CopyAndMove)
{
    static const char* STRINGA = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    UUID uuidA;
    {
        UUID uuidB = UUID::FromString(STRINGA);
        ASSERT_TRUE(uuidB.IsValid());
        uuidA = std::move(uuidB);
    }
    ASSERT_TRUE(uuidA.IsValid());
    ASSERT_EQ(uuidA.Size(), 16U);
    ASSERT_EQ(uuidA.ToString(false), STRINGA);
}

TEST(UUIDInit, UUIDFromString)
{
    static const char* STRINGA = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    UUID uuidA = UUID::FromString(STRINGA);
    ASSERT_TRUE(uuidA.IsValid());
    ASSERT_EQ(uuidA.Size(), 16U);
    ASSERT_EQ(uuidA.ToString(false), STRINGA);

    static const char* STRINGB = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    UUID uuidB = UUID::FromString(STRINGB);
    ASSERT_TRUE(uuidB.IsValid());
    ASSERT_EQ(uuidB.Size(), 16U);
    ASSERT_EQ(uuidB.ToString(false), STRINGB);

    static const char* STRINGZ = "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz";
    UUID uuidZ = UUID::FromString(STRINGZ);
    ASSERT_FALSE(uuidZ.IsValid());
    ASSERT_EQ(uuidZ.Size(), 0);
    ASSERT_EQ(uuidZ.ToString(false), "00000000000000000000000000000000");
}

TEST(UUIDComparisons, Compare)
{
    static const char* STRINGA = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    UUID uuidA = UUID::FromString(STRINGA);
    static const char* STRINGB = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    UUID uuidB = UUID::FromString(STRINGB);

    ASSERT_EQ(uuidA, uuidA);
    ASSERT_NE(uuidA, uuidB);
    ASSERT_TRUE(uuidA < uuidB);
    ASSERT_TRUE(uuidB > uuidA);
}

TEST(UUIDComparisons, Sorting)
{
    static const char* STRINGA = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    UUID uuidA = UUID::FromString(STRINGA);
    static const char* STRINGB = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    UUID uuidB = UUID::FromString(STRINGB);
    static const char* STRINGC = "cccccccccccccccccccccccccccccccc";
    UUID uuidC = UUID::FromString(STRINGC);
    static const char* STRINGD = "dddddddddddddddddddddddddddddddd";
    UUID uuidD = UUID::FromString(STRINGD);

    std::vector<UUID> v({uuidB, uuidD, uuidA, uuidC});
    std::sort(v.begin(), v.end());

    ASSERT_TRUE(v[0] < v[1]);
    ASSERT_TRUE(v[1] < v[2]);
    ASSERT_TRUE(v[2] < v[3]);
}
