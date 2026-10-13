add_library(mafia_android SHARED
    native_main.cpp
    engine_bridge.cpp
    input_bridge.cpp)

target_include_directories(mafia_android PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/ijoy)

target_compile_features(mafia_android PRIVATE cxx_std_11)
target_link_libraries(mafia_android PRIVATE android log ijoy rw_data EGL GLESv3)
