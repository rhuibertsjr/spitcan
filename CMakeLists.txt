cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS src)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(COMPILE_OPTIONS "-Wno-unused-function" APPEND)

add_definitions(-DPVC_LOGGING=1)
add_definitions(-DPVC_ASSERT=1)

project(project-pvc)

