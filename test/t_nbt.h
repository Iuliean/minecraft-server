#ifndef T_NBT_H
#define T_NBT_H
#include <gtest/gtest.h>
#include <random>
#include <ranges>
#include <vector>
#include "DataTypes/nbt.h"


class nbt_list_test : public testing::Test
{
protected:
    nbt_list_test()
        : testing::Test(),
          m_device(),
          m_distribution(0,10000)
    {}
    ~nbt_list_test() override = default;

    void SetUp() override{}
    void TearDown() override{}

    int get_random() { return m_distribution(m_device); }

    std::random_device m_device;
    std::uniform_int_distribution<int> m_distribution;

    template<typename T>
    mc::nbt::list perform_insert_for_type()
    {
        using namespace mc::nbt;
        list list;
        const auto iter = list.begin();
        EXPECT_EQ(list.get_list_type(), tag_type::UNKNOWN);
        EXPECT_EQ(list.size(), 0);

        EXPECT_NO_THROW(list.insert(iter, T()));
        EXPECT_EQ(list.get_list_type(), get_tag_type(T()));
        EXPECT_EQ(list.size(), 1);
        EXPECT_TRUE(list.front().is<T>());

        return list;
    }

    template<typename InsertT, typename TestT>
    void perform_insert_wrong_type()
    {
        using namespace mc::nbt;

        list list = perform_insert_for_type<InsertT>();
        const auto iter = list.begin();
        const size_t prev_size = list.size();
        const tag_type prev_tag = list.get_list_type();

        EXPECT_THROW(list.insert(iter, TestT()), nbt_error);
        EXPECT_THROW(list.insert(iter, TestT()), nbt_error);
        EXPECT_EQ(prev_size, list.size());
        EXPECT_EQ(prev_tag, list.get_list_type());
    }

    template<typename T>
    mc::nbt::list perform_insert_multiple_values()
    {
        using namespace mc::nbt;
        list list;
        const auto iter = list.begin();
        const int size = get_random();

        EXPECT_EQ(list.get_list_type(), tag_type::UNKNOWN);
        EXPECT_EQ(list.size(), 0);
        EXPECT_NO_THROW(list.insert(iter, size, T()));
        EXPECT_EQ(list.get_list_type(), get_tag_type(T()));
        EXPECT_EQ(list.size(), size);

        for (const auto& value : list)
            EXPECT_TRUE(value.is<T>());
        return list;
    }

    template<typename InsertT, typename TestT>
    void perform_insert_wrong_type_after_multiple_insert()
    {
        using namespace mc::nbt;

        list list = perform_insert_multiple_values<InsertT>();
        const auto iter = list.begin();
        const size_t prev_size = list.size();
        const tag_type prev_tag = list.get_list_type();

        EXPECT_THROW(list.insert(iter, TestT()), nbt_error);
        EXPECT_EQ(prev_size, list.size());
        EXPECT_EQ(prev_tag, list.get_list_type());
    }
};

TEST_F(nbt_list_test, insertion_value)
{
    using namespace mc::nbt;

    perform_insert_for_type<Byte>();
    perform_insert_for_type<Short>();
    perform_insert_for_type<Int>();
    perform_insert_for_type<Long>();
    perform_insert_for_type<Float>();
    perform_insert_for_type<Double>();
    perform_insert_for_type<ByteArray>();
    perform_insert_for_type<String>();
    perform_insert_for_type<list>();
    perform_insert_for_type<compound>();
    perform_insert_for_type<IntArray>();
    perform_insert_for_type<LongArray>();

    perform_insert_wrong_type<Byte, Short>();
    perform_insert_wrong_type<Byte, Int>();
    perform_insert_wrong_type<Byte, Long>();
    perform_insert_wrong_type<Byte, Float>();
    perform_insert_wrong_type<Byte, Double>();
    perform_insert_wrong_type<Byte, ByteArray>();
    perform_insert_wrong_type<Byte, String>();
    perform_insert_wrong_type<Byte, list>();
    perform_insert_wrong_type<Byte, compound>();
    perform_insert_wrong_type<Byte, IntArray>();
    perform_insert_wrong_type<Byte, LongArray>();

}

TEST_F(nbt_list_test, insertion_multiple_values)
{
    using namespace mc::nbt;

    perform_insert_multiple_values<Byte>();
    perform_insert_multiple_values<Short>();
    perform_insert_multiple_values<Int>();
    perform_insert_multiple_values<Long>();
    perform_insert_multiple_values<Float>();
    perform_insert_multiple_values<Double>();
    perform_insert_multiple_values<ByteArray>();
    perform_insert_multiple_values<String>();
    perform_insert_multiple_values<list>();
    perform_insert_multiple_values<compound>();
    perform_insert_multiple_values<IntArray>();
    perform_insert_multiple_values<LongArray>();

    perform_insert_wrong_type_after_multiple_insert<Byte, Short>();
    perform_insert_wrong_type_after_multiple_insert<Byte, Int>();
    perform_insert_wrong_type_after_multiple_insert<Byte, Long>();
    perform_insert_wrong_type_after_multiple_insert<Byte, Float>();
    perform_insert_wrong_type_after_multiple_insert<Byte, Double>();
    perform_insert_wrong_type_after_multiple_insert<Byte, ByteArray>();
    perform_insert_wrong_type_after_multiple_insert<Byte, String>();
    perform_insert_wrong_type_after_multiple_insert<Byte, list>();
    perform_insert_wrong_type_after_multiple_insert<Byte, compound>();
    perform_insert_wrong_type_after_multiple_insert<Byte, IntArray>();
    perform_insert_wrong_type_after_multiple_insert<Byte, LongArray>();

}

TEST_F(nbt_list_test, insert_iterators)
{
    using namespace mc::nbt;
    {

        list list;

        auto to_insert = std::ranges::to<std::vector<Int>>(std::views::iota(0, get_random()));


        EXPECT_NO_THROW(list.insert(list.begin(), to_insert.begin(), to_insert.end()));

        EXPECT_EQ(list.size(), to_insert.size());
        EXPECT_EQ(list.get_list_type(), tag_type::INT);

        for(const auto& value : list)
            EXPECT_TRUE(value.is<Int>());

        auto round2_insert = std::ranges::to<std::vector<long>>(std::views::iota(0, get_random()));

        EXPECT_THROW(list.insert(list.begin(), round2_insert.begin(), round2_insert.end()), nbt_error);

        EXPECT_EQ(list.size(), to_insert.size());
        EXPECT_EQ(list.get_list_type(), tag_type::INT);

        for(const auto& value : list)
            EXPECT_TRUE(value.is<Int>());
    }

    //We now treat the case for generig tags
    //We need to make sure they are the same
    // and that mismatched types are not allowed
    {
        list list;
        std::vector<tag> to_insert = {int(123), int(12356), int(-123)};

        EXPECT_NO_THROW(list.insert(list.begin(), to_insert.begin(), to_insert.end()));

        EXPECT_EQ(list.size(), to_insert.size());
        EXPECT_EQ(list.get_list_type(), tag_type::INT);

        for(const auto& value : list)
            EXPECT_TRUE(value.is<Int>());

        std::vector<tag> round2_insert = {int(123), float(12356), double(-123)};

        EXPECT_THROW(list.insert(list.begin(), round2_insert.begin(), round2_insert.end()), nbt_error);

        EXPECT_EQ(list.size(), to_insert.size());
        EXPECT_EQ(list.get_list_type(), tag_type::INT);

        for(const auto& value : list)
            EXPECT_TRUE(value.is<Int>());
    }

}

TEST_F(nbt_list_test, insert_range)
{
    using namespace mc::nbt;

    
}

TEST_F(nbt_list_test, emplace)
{
    using namespace mc::nbt;

    list l;
    l.set_list_type(tag_type::INT);
    EXPECT_NO_THROW(l.emplace(l.begin(), 123));
    EXPECT_NO_THROW(l.emplace(l.begin(), 123));
    EXPECT_THROW(l.emplace(l.begin(), 123, 122), nbt_error);

    EXPECT_NO_THROW(l.push_back( 123));

}

#endif