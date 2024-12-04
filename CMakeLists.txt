cmake_minimum_required(VERSION 3.16)

project(oxc)

include("${CMAKE_CURRENT_SOURCE_DIR}/CPM.cmake")
CPMAddPackage(
	NAME boost_multiprecision
	URL "https://github.com/boostorg/multiprecision/archive/refs/tags/Boost_1_86_0.zip"
	URL_HASH SHA256=855bd78a1a4ee18bfb385aca54c153b55e4c97a137604be951208b144df7e3bd
)
add_library(${PROJECT_NAME} STATIC oxc.cpp)

target_link_libraries(${PROJECT_NAME} boost_multiprecision)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
