cmake_minimum_required(VERSION 3.12)
project(lang VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_COMPILER "clang++")

option (USE_BOOST_FILESYSTEM "Use boost filesystem" OFF)

configure_file("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_SOURCE_DIR}/config.h")

# cmake builds extensions
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/third_party/sanitizers-cmake/cmake" ${CMAKE_MODULE_PATH})
find_package(Sanitizers)

# libraries
add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE "third_party/doctest/doctest")

add_library(cxxopts INTERFACE)
target_include_directories(cxxopts INTERFACE "third_party/cxxopts/include")

if (USE_BOOST_FILESYSTEM)
    find_package(Boost 1.67 REQUIRED COMPONENTS filesystem)
    set (EXTRA_LIBS ${EXTRA_LIBS} Boost::filesystem)
endif (USE_BOOST_FILESYSTEM)

add_subdirectory("compiler")
add_subdirectory("test/unit")
add_subdirectory("test/snapshot")

add_executable(lang main.cc)
target_compile_options(lang PRIVATE -Wall)
target_compile_features(lang PRIVATE cxx_std_17)
target_include_directories(lang PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(lang compiler doctest ${EXTRA_LIBS})
add_sanitizers(lang)

