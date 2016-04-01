cmake_minimum_required(VERSION 3.1)
project(activity-monitor)

find_package(Boost COMPONENTS system thread regex REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

FIND_PACKAGE ( Threads REQUIRED )


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -w")

set (EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT} pthread mongocxx mongoclient boost_thread boost_system boost_regex X11 xcb Xmu )
set(SOURCE_FILES main.cpp xutils/XWatcherDaemon.cpp xutils/XWatcherDaemon.h)
include_directories(/usr/include/mongocxx/v0.3/)
include_directories(/usr/include/bsoncxx/v0.3/)

add_executable(activity-monitor ${SOURCE_FILES})
target_link_libraries (activity-monitor ${EXTRA_LIBS})