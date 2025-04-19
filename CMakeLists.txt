cmake_minimum_required(VERSION 3.30)
project(MemoriaCache)

set(CMAKE_CXX_STANDARD 20)

add_executable(MemoriaCache main.cpp
        model/cpp/DataLine.cpp
        model/h/DataLine.h
        model/cpp/CacheSet.cpp
        model/h/CacheSet.h
        model/cpp/Dram.cpp
        model/h/Dram.h)
