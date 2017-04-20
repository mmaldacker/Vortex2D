#include <gtest/gtest.h>
#include "glfw.h"

int main(int argc, char **argv)
{
    GLFWApp glfw(1000, 1000, false);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
