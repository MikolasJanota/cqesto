cmake_minimum_required(VERSION 3.24)
project(cqesto)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(Helpers)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Git)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Default build type: Release" FORCE)
endif()

option(USE_CADICAL "Use cadical as the base solver" OFF)
option(USE_CMS "Use cms as the base solver" OFF)
option(STATIC_BINARY "Compile static" OFF)
option(PROFILING "Compile with profiling support" OFF)

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

include_directories(src)

file(GLOB CPPSOURCES "src/*.cpp")
file(GLOB CSOURCES "src/*.c")
list(APPEND CPPSOURCES "${CMAKE_CURRENT_BINARY_DIR}/version.cpp")
 
add_executable(${CMAKE_PROJECT_NAME} ${CPPSOURCES} ${CSOURCES})

if (PROFILING)
  add_check_c_cxx_flag("-pg")
  add_check_c_cxx_flag("-ggdb3")
endif()

if (STATIC_BINARY)
  set(ZLIB_USE_STATIC_LIBS "ON")
  target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC "-static")
endif()

# CMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -pg"
# CMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS} -pg"
# CMAKE_SHARED_LINKER_FLAGS="${CMAKE_SHARED_LINKER_FLAGS} -pg"

find_package(ZLIB)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ZLIB::ZLIB)

if (USE_CADICAL)
  # even though we are not using minisat, we use its literals to communicate it with ipasir
  include_directories(contrib/minisat/include) 
  message(STATUS "Using cadical")
  include_directories(contrib/cadical/)

  find_library(CADICAL_LIBRARY
    NAMES cadical
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/cadical/"
    )
  if(NOT CADICAL_LIBRARY)
    message(FATAL_ERROR "CADICAL library not found")
  else()
    message(STATUS "Found ${CADICAL_LIBRARY}")
  endif()
  add_library(cadical STATIC IMPORTED GLOBAL)
  set_target_properties(cadical PROPERTIES IMPORTED_LOCATION ${CADICAL_LIBRARY})

  target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE USE_IPASIR=1)
  target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC cadical)
elseif(USE_CMS)
  message(STATUS "Using cms")
  include_directories(contrib/cms/include)
  if (STATIC_BINARY)
    find_library(CMS_LIBRARY
      NAMES libcryptominisat5.a cryptominisat5
      HINTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/cms/lib"
      )
  else()
    find_library(CMS_LIBRARY
      NAMES libcryptominisat5.so cryptominisat5
      HINTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/cms/lib"
      )
  endif()
  if(NOT CMS_LIBRARY)
    message(FATAL_ERROR "CMS library not found")
  else()
    message(STATUS "Found ${CMS_LIBRARY}")
  endif()
  if (STATIC_BINARY)
    add_library(cms STATIC IMPORTED GLOBAL)
  else()
    add_library(cms SHARED IMPORTED GLOBAL)
  endif()
  set_target_properties(cms PROPERTIES IMPORTED_LOCATION ${CMS_LIBRARY})
  target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE USE_CMS=1)
  target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC cms)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC Threads::Threads)
else()
  include_directories(contrib/minisat/include)
  message(STATUS "Using minisat")

  # find_library(MINISAT_LIBRARY
  #   NAMES libminisat.a minisat
  #   HINTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/minisat/lib"
  #   )
  if (STATIC_BINARY)
    find_library(MINISAT_LIBRARY
      NAMES libminisat.a minisat
      HINTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/minisat/lib"
      )
  else()
    find_library(MINISAT_LIBRARY
      NAMES libminisat.so minisat
      HINTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/minisat/lib"
      )
  endif()

  if(NOT MINISAT_LIBRARY)
    message(FATAL_ERROR "MINISAT library not found")
  else()
    message(STATUS "Found ${MINISAT_LIBRARY}")
  endif()

  # add_library(minisat STATIC IMPORTED GLOBAL)

  if (STATIC_BINARY)
    add_library(minisat STATIC IMPORTED GLOBAL)
  else()
    add_library(minisat SHARED IMPORTED GLOBAL)
  endif()

  set_target_properties(minisat PROPERTIES IMPORTED_LOCATION ${MINISAT_LIBRARY})

  target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE USE_MINISAT=1)
  target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC minisat)
endif()

message(STATUS "Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "CXXFLAGS   : ${CMAKE_CXX_FLAGS}")
