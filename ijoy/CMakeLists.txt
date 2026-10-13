cmake_minimum_required(VERSION 3.29)

project(ijoy)

if (ANDROID)
    # DirectInput is Windows-only. The Android target preserves the IJoy shared
    # library boundary and will be backed by native Android input events.
    add_library(ijoy SHARED android_ijoy.cpp)
    set_target_properties(ijoy PROPERTIES OUTPUT_NAME IJoy)
    target_include_directories(ijoy PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/common
        ${CMAKE_SOURCE_DIR}/android)
    target_compile_features(ijoy PRIVATE cxx_std_11)
    target_link_libraries(ijoy PRIVATE android log)
    return()
endif()

# The original is a single translation unit
add_library(ijoy SHARED ijoy.cpp)
# The export directory names the DLL
set_target_properties(ijoy PROPERTIES OUTPUT_NAME IJoy)

# The DirectX 8.1 SDK: its dinput.lib and dxguid.lib objects are in the Rich header (Implib700 9210, Utc13 9178).
# Its headers and libraries come before the compiler's own, which are DirectX 6's. The SDK ships an old basetsd.h,
# so newer compilers need the Windows SDK's headers first.
if (MSVC_VERSION GREATER 1500)
    target_include_directories(ijoy PRIVATE $ENV{WindowsSdkDir}/Include/$ENV{WindowsSDKVersion}/shared)
endif()
target_include_directories(ijoy PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/dx81sdk/include)
target_link_directories(ijoy PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/dx81sdk/lib)

if (MSVC_VERSION EQUAL 1200)
    target_compile_options(ijoy PRIVATE /ML /GX)
    set(ijoy_cl "${CMAKE_CURRENT_BINARY_DIR}/cl")
    file(COPY "${CMAKE_SOURCE_DIR}/3rdparty/msvc600/VC98/Bin/CL.EXE" "${CMAKE_SOURCE_DIR}/3rdparty/msvc600/VC98/Bin/C1XX.DLL"
        "${CMAKE_SOURCE_DIR}/3rdparty/msvc600/VC98/Bin/MSPDB60.DLL"
        "${CMAKE_SOURCE_DIR}/3rdparty/msvc600_sp5_vcpp/VC98/Bin/c2.dll" DESTINATION "${ijoy_cl}")
    set_target_properties(ijoy PROPERTIES
        CXX_COMPILER_LAUNCHER "${MAFIA_PYTHON};${CMAKE_SOURCE_DIR}/tools/clswap.py;${ijoy_cl}")
    set(CMAKE_CXX_USING_LINKER_VC6RTM "${CMAKE_SOURCE_DIR}/3rdparty/msvc600/VC98/Bin/LINK.EXE")
    set_target_properties(ijoy PROPERTIES LINKER_TYPE VC6RTM)
    target_link_directories(ijoy PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/msvc600/VC98/Lib)
    target_link_libraries(ijoy PRIVATE kernel32 user32 dinput dxguid)
    postlink(ijoy --timestamp 0x3d35e3d1)
else()
    target_link_libraries(ijoy PRIVATE user32 dinput dxguid)
endif()

vc6_header_dependencies(ijoy)
