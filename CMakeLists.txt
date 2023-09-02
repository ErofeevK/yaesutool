cmake_minimum_required(VERSION 3.25)

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0092 NEW)

project(
  yaesutool
  VERSION 1.0.0.0
  LANGUAGES C)

if(MINGW32)
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_definitions(MINGW32)
else()
    # additional warnings
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()


add_executable(${PROJECT_NAME} main.c util.c radio.c ft-60.c vx-2.c util.h radio.h)
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DVERSION=\"${PROJECT_VERSION}\"")
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)
