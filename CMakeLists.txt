cmake_minimum_required(VERSION 3.24)
project(conv_event CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)
include_directories(ErrorLog)

add_executable(conv_event main.cpp
        ErrorLog/ErrorLog.h
        conv.h conn.h threadpool.h threadpool.h threadpool.h)
