cmake_minimum_required(VERSION 3.25)

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0092 NEW)

file(STRINGS version.txt VER LIMIT_COUNT 1)


project(
  yaesutool
  VERSION ${VER}
  LANGUAGES C)

if(MINGW32)
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_definitions(MINGW32)
else()
    # additional warnings
    add_compile_options(-Wall -Wextra -Wpedantic)
if(BUILD_STATIC)
    link_libraries("-static")
endif()
endif()


set(GIT_COMMIT_SHA "")

# Check if git is available
find_program(GIT_EXECUTABLE git)
if(GIT_EXECUTABLE)
    # Check if the current directory is a git repository
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --is-inside-work-tree
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE GIT_REPO_CHECK
    )

    if(GIT_REPO_CHECK EQUAL 0)
        # Get the short version of the current Git commit SHA
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_SHA
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        # Prepend a dash to the SHA
        set(GIT_COMMIT_SHA "-${GIT_COMMIT_SHA}")
    endif()
endif()


add_executable(${PROJECT_NAME} main.c util.c radio.c ft-60.c vx-2.c util.h radio.h)
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DVERSION=\"${PROJECT_VERSION}${GIT_COMMIT_SHA}\"")
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)


if(MINGW32)
set(CPACK_GENERATOR "ZIP")
install(PROGRAMS build/${PROJECT_NAME}.exe DESTINATION bin)
else()
set(CPACK_GENERATOR "TGZ")
install(PROGRAMS build/${PROJECT_NAME} DESTINATION bin)

if(BUILD_STATIC)
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-Linux-static")
endif()

endif()
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
                      
include(CPack)






