cmake_minimum_required(VERSION 3.12)
project(spl_assignment_3_client)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()


add_executable(spl_assignment_3_client src/main.cpp include/ConnectionHandler.h src/ConnectionHandler.cpp include/Task.h src/Task.cpp)
target_link_libraries(my_target)