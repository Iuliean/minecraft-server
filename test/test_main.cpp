#include <gtest/gtest.h>

#include "t_nbt.h"

int main(int argc, char** argv)
{
    try
    {
        testing::InitGoogleTest(&argc, argv);

        return RUN_ALL_TESTS();
    }
    catch(const std::exception& ex)
    {
        std::cerr << ex.what();
        return -1;
    }

    return 0;
}