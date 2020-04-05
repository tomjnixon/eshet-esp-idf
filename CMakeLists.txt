idf_component_register(SRCS "src/eshet_esp.cpp" INCLUDE_DIRS "include" REQUIRES
                       esp_common)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(eshetcpp)
target_link_libraries(${COMPONENT_LIB} PUBLIC eshetcpp)
target_compile_options(${COMPONENT_LIB} PUBLIC -DACTORPP_RECV_THREAD_SHUTDOWN)

