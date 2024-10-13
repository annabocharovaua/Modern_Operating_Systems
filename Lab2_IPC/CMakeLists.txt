cmake_minimum_required(VERSION 3.22)
project(Lab2_IPC C)

set(CMAKE_C_STANDARD 17)

add_executable(Lab2_IPC main.c
        FileIO.h
        MmapIO.h
        SharedIO.h
        config.h
)
