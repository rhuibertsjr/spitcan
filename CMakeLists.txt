cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS src)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

add_definitions(-DPVC_LOGGING=1)
add_definitions(-DPVC_ASSERT=1)

project(project-pvc)

