cmake_minimum_required(VERSION 3.29)

project(mafia)

include_directories(common)

function(vc6_header_dependencies target)
    if (MSVC_VERSION EQUAL 1200)
        file(GLOB_RECURSE headers CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_SOURCE_DIR}/common/*.h")
        get_target_property(sources ${target} SOURCES)
        set_source_files_properties(${sources} PROPERTIES OBJECT_DEPENDS "${headers}")
    endif()
endfunction()

# Android is a separate portability build. Keep the original Windows
# reconstruction targets untouched while bringing portable components up one
# at a time. rw_data is native; ijoy currently uses an Android input shim.
if (ANDROID)
    add_subdirectory(rw_data)
    add_subdirectory(ijoy)
    return()
endif()

find_program(MAFIA_PYTHON NAMES python python3 HINTS "${CMAKE_SOURCE_DIR}/.venv/Scripts" REQUIRED)
function(postlink target)
    if ((MSVC_VERSION EQUAL 1200 OR MSVC_VERSION EQUAL 1500) AND CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${MAFIA_PYTHON}" "${CMAKE_SOURCE_DIR}/tools/postlink.py" "$<TARGET_FILE:${target}>" ${ARGN}
            VERBATIM)
    endif()
endfunction()

if (MSVC_VERSION EQUAL 1200 AND CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set_property(GLOBAL APPEND PROPERTY JOB_POOLS vc6_compile=1)
    set(CMAKE_JOB_POOL_COMPILE vc6_compile)
endif()

if (NOT MSVC_VERSION EQUAL 1500)
    add_subdirectory(rw_data)
    add_subdirectory(ijoy)
endif()
add_subdirectory(setup)

if (MSVC_VERSION EQUAL 1200 AND CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/reccmp-build.yml" CONTENT
"project: ${CMAKE_SOURCE_DIR}\ntargets:\n  RWDATA:\n    path: $<TARGET_FILE:rw_data>\n    pdb: $<TARGET_PDB_FILE:rw_data>\n  SETUP:\n    path: $<TARGET_FILE:setup>\n    pdb: $<TARGET_PDB_FILE:setup>\n  IJOY:\n    path: $<TARGET_FILE:ijoy>\n    pdb: $<TARGET_PDB_FILE:ijoy>\n")
elseif (MSVC_VERSION EQUAL 1500)
    file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/reccmp-build.yml" CONTENT
"project: ${CMAKE_SOURCE_DIR}\ntargets:\n  SETUPSTEAM:\n    path: $<TARGET_FILE:setup>\n    pdb: $<TARGET_PDB_FILE:setup>\n")
endif()
