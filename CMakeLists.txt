idf_component_register()

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake") 

add_subdirectory(eshetcpp)
target_link_libraries(${COMPONENT_LIB} INTERFACE eshetcpp)
target_compile_options(${COMPONENT_LIB} INTERFACE -DACTORPP_RECV_THREAD_SHUTDOWN)

