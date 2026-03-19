cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(env_mon_zephyr)

target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/util
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sensors
  ${CMAKE_CURRENT_SOURCE_DIR}/src/control
  ${CMAKE_CURRENT_SOURCE_DIR}/src/storage
  ${CMAKE_CURRENT_SOURCE_DIR}/src/watchdog
  ${CMAKE_CURRENT_SOURCE_DIR}/src/misc
)

target_sources(app PRIVATE
  src/main.c
  src/util/avg_list.c
  src/sensors/temperature.c
  src/sensors/humidity.c
  src/sensors/light.c
  src/sensors/pressure.c
  src/control/sys_control.c
  src/storage/storage.c
  src/watchdog/wdt.c
  src/misc/misc.c
)
