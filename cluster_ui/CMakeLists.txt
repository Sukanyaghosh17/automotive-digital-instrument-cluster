cmake_minimum_required(VERSION 3.16)

project(ClusterUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Gui)
find_package(vsomeip3 REQUIRED)
find_package(Threads REQUIRED)

qt_standard_project_setup()

qt_add_executable(cluster_ui
    src/main.cpp
    src/ClusterBackend.cpp
    src/ClusterWorker.cpp
    src/TripComputer.cpp
    include/ClusterBackend.hpp
    include/ClusterWorker.hpp
    include/TripComputer.hpp
)

qt_add_qml_module(cluster_ui
    URI ClusterUI
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_include_directories(cluster_ui PRIVATE
    include
    ${VSOMEIP3_INCLUDE_DIRS}
)

target_link_libraries(cluster_ui
    PRIVATE
    Qt6::Quick
    Qt6::Core
    Qt6::Gui
    vsomeip3
    Threads::Threads
)
