# Copyright (C) 2021 by the INTELLI team (https://github.com/OoOJoin)
# Copyright (C) 2021 by the INTELLI team (https://github.com/intellistream)

cmake_minimum_required(VERSION 3.14)
project(IntelliStream CXX)

# Judge the os system


# Custom CMake find instructions and macros
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
include(cmake/macros.cmake)
include(cmake/default.cmake)


# C++ Standard
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#gcc 10 g++10
message(STATUS "sudo add-apt-repository 'deb http://mirrors.kernel.org/ubuntu hirsute main universe'")
message(STATUS "sudo apt-get update")
message(STATUS "sudo apt install gcc-11 g++-11")
message(STATUS "sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 11")
message(STATUS "sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 11")

# Set Optimization Flags
set(CMAKE_CXX_FLAGS "-std=c++20 -Wall -Werror=return-type -Werror=unused-variable -Werror=unused-parameter")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DNO_RACE_CHECK -DIntelliStream_DEBUG_MODE=1")
set(CMAKE_CXX_FLAGS_RELEASE "-Wno-ignored-qualifiers -Wno-sign-compare -O3")
set(OOO_JOIN_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/lib)

add_definitions("-Wall -g")

# Set LOGGING_LEVEL Flag
if (IntelliStream_LOGGING_LEVEL)
    get_log_level_value(IntelliStream_LOGGING_VALUE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIntelliStream_LOGGING_LEVEL=${IntelliStream_LOGGING_VALUE}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DIntelliStream_LOGGING_LEVEL=${IntelliStream_LOGGING_VALUE}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DIntelliStream_LOGGING_LEVEL=${IntelliStream_LOGGING_VALUE}")
else (IntelliStream_LOGGING_LEVEL)
    message("---Everything will be logged")
endif (IntelliStream_LOGGING_LEVEL)

message(STATUS "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE}")


option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")

# Print all used include directories
message(STATUS "INCLUDE_DIRS:")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach (dir ${dirs})
    message(STATUS "  - ${dir}")
endforeach ()

message(STATUS " Libraries: ${LIBRARIES}")

# Add Source Code
add_subdirectory(src)

# Add Library
get_sources(IntelliStream_SOURCE_FILES)
get_headers(IntelliStream_HEADER_FILES)

# 这行是针对我的机子，其他人运行的时候删了
#set(Torch_DIR /usr/local/libtorch/share/cmake/Torch)
find_package(Torch REQUIRED)

message(STATUS "TORCH_INCLUDE_DIRS: ${TORCH_INCLUDE_DIRS}")
message(STATUS "TORCH_LIBRARIES: ${TORCH_LIBRARIES}")

add_library(IntelliStreamOoOJoin SHARED ${IntelliStream_SOURCE_FILES} ${IntelliStream_HEADER_FILES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(IntelliStreamOoOJoin PUBLIC "include" ${TORCH_INCLUDE_DIRS})
target_link_libraries(IntelliStreamOoOJoin PUBLIC ${LIBRARIES} ${TORCH_LIBRARIES})


include_directories(${OOO_JOIN_INCLUDE_DIR})

if (MSVC)
    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
    add_custom_command(TARGET example-app
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${TORCH_DLLS}
            $<TARGET_FILE_DIR:example-app>)
endif (MSVC)
#Add benchmarks with command
add_subdirectory(benchmark)

#Add tests (based on GTest suits)
if (ENABLE_UNIT_TESTS)
    add_subdirectory(test)
endif (ENABLE_UNIT_TESTS)

install(DIRECTORY "include" DESTINATION "/IntelliStream" COMPONENT IntelliStreamOoOJoin)
# copy scripts
file(GLOB allCopyFiles "${PROJECT_SOURCE_DIR}/scripts/*")
file(COPY ${allCopyFiles} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/benchmark/scripts)
file(COPY ${allCopyFiles} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/benchmark/scripts)
#file(COPY ${allCopyFiles} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/scripts)
