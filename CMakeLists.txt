cmake_minimum_required(VERSION 3.25)
project(TapRoot C)

set(CMAKE_C_STANDARD 23)
#set(CMAKE_C_FLAGS "-fsanitize=undefined -fsanitize=thread -fno-omit-frame-pointer")

enable_testing()

add_library(TapRoot taproot/taproot.c taproot/taproot.h)

add_executable(TapRootTest Test/main.c taproot/taproot.c taproot/taproot.h)

add_test(
        NAME TestTapRoot
        COMMAND TapRootTest
)