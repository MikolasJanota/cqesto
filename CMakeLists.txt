cmake_minimum_required(VERSION 3.10)
project(cqesto)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Git)
find_package(ZLIB)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Default build type: Release" FORCE)
endif()

option(USE_CADICAL "Use cadical as the base solver" OFF)

# Add sanitizer flags for C++ compiler for "Debug" configuration
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined,bool")

## https://stackoverflow.com/questions/1435953/how-can-i-pass-git-sha1-to-compiler-as-definition-using-cmake
# the commit's SHA1, and whether the building workspace was dirty or not
execute_process(COMMAND
  "${GIT_EXECUTABLE}" describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_SHA1
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# the date of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_DATE
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# the subject of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" log -1 --format=%s
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# generate version.cpp
configure_file("src/version.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" @ONLY)

#######

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(contrib/minisat/include)
include_directories(src)

file(GLOB CPPSOURCES "src/*.cpp")
file(GLOB CSOURCES "src/*.c")
list(APPEND CPPSOURCES "${CMAKE_CURRENT_BINARY_DIR}/version.cpp")
 
add_executable(${CMAKE_PROJECT_NAME} ${CPPSOURCES} ${CSOURCES})

if (USE_CADICAL)
  message(STATUS "Using cadical")
  include_directories(contrib/cadical)
  add_library(cadical STATIC IMPORTED GLOBAL)
  target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE USE_IPASIR=1)
  target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC cadical)
  set_target_properties(cadical PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/contrib/cadical/libcadical.a)
else()
  message(STATUS "Using minisat")
  target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE USE_MINISAT=1)

  add_library(minisat STATIC IMPORTED GLOBAL)
  set_target_properties(minisat PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/contrib/minisat/lib/libminisat.a
  )
  # set_target_properties(minisat PROPERTIES
  #   IMPORTED_LOCATION_RELEASE
  #   ${CMAKE_CURRENT_SOURCE_DIR}/contrib/minisat/lib/libminisat.a
  #   IMPORTED_LOCATION_DEBUG
  #   ${CMAKE_CURRENT_SOURCE_DIR}/contrib/minisat/minisat/build/debug/lib/libminisat.a
  # )

  target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC minisat)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ZLIB::ZLIB)

message(STATUS "Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "CXXFLAGS   : ${CMAKE_CXX_FLAGS}")
