cmake_minimum_required(VERSION 3.25)
project(TapRoot C)

set(CMAKE_C_STANDARD 23)

add_executable(TapRoot main.c taproot/taproot.c taproot/taproot.h)
