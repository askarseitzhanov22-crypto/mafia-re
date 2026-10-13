cmake_minimum_required(VERSION 3.25)

project(rw_data)

add_library(rw_data SHARED rw_data.cpp)

target_include_directories(rw_data PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/common
)

if (ANDROID)
    target_include_directories(rw_data PRIVATE ${CMAKE_SOURCE_DIR}/android)
    target_compile_features(rw_data PRIVATE cxx_std_11)
    target_link_libraries(rw_data PRIVATE log)
else()
    # Original VC6 build flags and Windows import libraries.
    if (MSVC_VERSION EQUAL 1200)
        target_compile_options(rw_data PRIVATE /ML /GX /Gz)
        target_link_libraries(rw_data PRIVATE kernel32 user32)
        postlink(rw_data --timestamp 0x3d2e1e08)
    endif()
endif()

vc6_header_dependencies(rw_data)
