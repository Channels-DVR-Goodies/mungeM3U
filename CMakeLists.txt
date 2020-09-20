cmake_minimum_required(VERSION 3.17)
project(mungeM3U C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(mungeM3U
        dictionary.c dictionary.h
        mungeM3U.c mungeM3U.h
        keywords.h
        patterns.h)
