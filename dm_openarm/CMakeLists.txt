cmake_minimum_required(VERSION 3.16)
project(dm_openarm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0)
pkg_check_modules(LIBUDEV REQUIRED IMPORTED_TARGET libudev)
find_package(yaml-cpp REQUIRED)

include(CTest)

option(DM_OPENARM_BUILD_PYTHON "Build Python bindings" OFF)
option(DM_OPENARM_BUILD_EXAMPLES "Build dm_openarm examples" ON)

add_library(
  damiao_sdk STATIC
  third_party/damiao_sdk/src/protocol/damiao.cpp
)
target_include_directories(
  damiao_sdk
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/damiao_sdk/include
)
target_link_libraries(
  damiao_sdk
  PUBLIC
    Threads::Threads
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/damiao_sdk/lib/libu2canfd.a
    PkgConfig::LIBUSB
    PkgConfig::LIBUDEV
)
set_target_properties(damiao_sdk PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(
  dm_openarm
  src/backend/dm_serial_backend.cpp
  src/dm_arm.cpp
  src/mit_control_command.cpp
  src/mit_loop_controller.cpp
  src/motor.cpp
  src/motor_group.cpp
  src/yaml_loader.cpp
)
target_include_directories(
  dm_openarm
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(
  dm_openarm
  PUBLIC
    damiao_sdk
    yaml-cpp
)
set_target_properties(dm_openarm PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(DM_OPENARM_BUILD_EXAMPLES)
  add_executable(check_comm examples/check_comm.cpp)
  target_link_libraries(check_comm PRIVATE dm_openarm)

  add_executable(enable_disable examples/enable_disable.cpp)
  target_link_libraries(enable_disable PRIVATE dm_openarm)

  add_executable(hold_position examples/hold_position.cpp)
  target_link_libraries(hold_position PRIVATE dm_openarm)

  add_executable(set_zero_position examples/set_zero_position.cpp)
  target_link_libraries(set_zero_position PRIVATE dm_openarm)

  add_executable(arm_mit_control examples/arm_mit_control.cpp)
  target_link_libraries(arm_mit_control PRIVATE dm_openarm)
endif()

if(DM_OPENARM_BUILD_PYTHON)
  find_package(
    Python
    COMPONENTS Interpreter Development.Module
    REQUIRED)
  execute_process(
    COMMAND ${Python_EXECUTABLE} -m nanobind --cmake_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE nanobind_ROOT)
  find_package(nanobind CONFIG REQUIRED)

  nanobind_add_module(dm_openarm_python src/python_bindings.cpp)
  set_target_properties(dm_openarm_python PROPERTIES OUTPUT_NAME "_core")
  target_link_libraries(dm_openarm_python PRIVATE dm_openarm)
  install(TARGETS dm_openarm_python LIBRARY DESTINATION dm_openarm)
endif()

if(BUILD_TESTING)
  add_executable(test_yaml_loader tests/test_yaml_loader.cpp)
  target_link_libraries(test_yaml_loader PRIVATE dm_openarm)
  add_test(NAME test_yaml_loader COMMAND test_yaml_loader)

  add_executable(test_mit_control_command tests/test_mit_control_command.cpp)
  target_link_libraries(test_mit_control_command PRIVATE dm_openarm)
  add_test(NAME test_mit_control_command COMMAND test_mit_control_command)

  add_executable(test_mit_loop_controller tests/test_mit_loop_controller.cpp)
  target_link_libraries(test_mit_loop_controller PRIVATE dm_openarm)
  add_test(NAME test_mit_loop_controller COMMAND test_mit_loop_controller)
endif()
