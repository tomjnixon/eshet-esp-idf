if(IDF_TARGET)
  idf_component_register(
    SRCS "src/eshet_esp.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_common app_update)

  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

  add_subdirectory(eshetcpp)
  target_link_libraries(${COMPONENT_LIB} PUBLIC eshetcpp)
  target_compile_options(${COMPONENT_LIB} PUBLIC -DACTORPP_RECV_THREAD_SHUTDOWN)

  # vrefbuffer.hpp is not necessary, but defines iovec, which conflicts with the
  # esp-idf definition
  target_compile_options(${COMPONENT_LIB} PUBLIC -DMSGPACK_VREFBUFFER_HPP)

  # add custom commands for ota flashing to the given path. Call this after
  # project(), like setup_ota(/path/to/ota), and use ESHET_OTA_PATH in the main
  # component to access the same path.
  function(setup_ota ota_path)
    add_custom_target(
      ota_flash
      DEPENDS ${PROJECT_BIN}
      COMMAND eshet_ota flash ${ota_path} ${PROJECT_BIN}
      USES_TERMINAL)

    add_custom_target(
      ota_mark_valid
      COMMAND eshet_ota mark_valid ${ota_path}
      USES_TERMINAL)

    add_custom_target(
      ota_mark_invalid
      COMMAND eshet_ota mark_invalid ${ota_path}
      USES_TERMINAL)

    get_target_property(main_alias idf::main ALIASED_TARGET)
    target_compile_definitions(${main_alias}
                               PUBLIC "ESHET_OTA_PATH=\"${ota_path}\"")
  endfunction()
else()
  cmake_minimum_required(VERSION 3.9)
  project(
    eshet-esp-idf
    VERSION 0.0.1
    LANGUAGES CXX)

  add_subdirectory(eshetcpp)

  # XXX: hack; should be allowed to use utils properly when using eshetcpp as a
  # subproject
  add_executable(eshet_ota src/eshet_ota_cli.cpp eshetcpp/src/utils.cpp)
  target_link_libraries(eshet_ota PRIVATE eshetcpp CLI11 rapidjson)
  target_include_directories(eshet_ota PRIVATE eshetcpp/src)

  install(TARGETS eshet_ota DESTINATION bin)
endif()
