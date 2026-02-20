# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS ${app_sources})

# Link GT30L24A3W font library
target_link_libraries(${COMPONENT_LIB} INTERFACE ${CMAKE_SOURCE_DIR}/src/components/cyberpi/src/lcd/libGT30L24A3W.a)
