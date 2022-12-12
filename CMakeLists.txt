cmake_minimum_required(VERSION 3.13)

add_library(pico_w_connection_manager INTERFACE)
target_sources(pico_w_connection_manager INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pico_w_connection_manager.cpp
)
target_include_directories(pico_w_connection_manager INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../parson
    ${CMAKE_CURRENT_LIST_DIR}/../littlefs-lib
)
# NOTE you must build the parson and littlefs-lib libraries in the project that uses this project
target_link_libraries(pico_w_connection_manager INTERFACE pico_cyw43_arch_lwip_threadsafe_background pico_stdlib littlefs-lib)
target_compile_options(pico_w_connection_manager INTERFACE -DRPPICOMIDI_PICO_W)

