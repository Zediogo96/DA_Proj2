cmake_minimum_required(VERSION 3.20)
project(DA_Travel)

set(CMAKE_CXX_STANDARD 14)

add_executable(DA_Travel main.cpp minHeap.h maxHeap.h graph.h graph.cpp Network.cpp Network.h Starter.cpp Starter.h Menu.cpp Menu.h)

