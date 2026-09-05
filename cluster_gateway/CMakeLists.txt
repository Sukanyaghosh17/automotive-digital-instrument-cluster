cmake_minimum_required(VERSION 3.16)

project(ClusterGateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(vsomeip3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread log)
find_package(Threads REQUIRED)

include_directories(
    include
    ${VSOMEIP3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

add_executable(cluster_gateway
    src/main.cpp
    src/ClusterGateway.cpp
    src/CANFrameDecoder.cpp
)

target_link_libraries(cluster_gateway
    PRIVATE
    vsomeip3
    ${Boost_LIBRARIES}
    Threads::Threads
)
