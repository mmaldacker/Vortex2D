#include "gtest/gtest.h"
#include "RenderWindow.h"
#include "GLFW.h"

int main(int argc, char **argv)
{
    GLFW glfw;
    RenderWindow mainWindow(1000, 1000, "Test Window");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
