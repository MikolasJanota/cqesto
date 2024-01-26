include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

# Add a C flag to the global list of C flags.
macro(add_c_flag flag)
  if(CMAKE_C_FLAGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
  else()
    set(CMAKE_C_FLAGS "${flag}")
  endif()
  message(STATUS "Configuring with C flag '${flag}'")
endmacro()

# Add a CXX flag to the global list of CXX flags.
macro(add_cxx_flag flag)
  if(CMAKE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
  else()
    set(CMAKE_CXX_FLAGS "${flag}")
  endif()
  message(STATUS "Configuring with CXX flag '${flag}'")
endmacro()

# Add a C and CXX flag to the global list of C/CXX flags.
macro(add_c_cxx_flag flag)
  add_c_flag(${flag})
  add_cxx_flag(${flag})
endmacro()

# Check if C flag is supported and add to global list of C flags.
macro(add_check_c_flag flag)
  string(REGEX REPLACE "[-=]" "_" flagname ${flag})
  check_c_compiler_flag("${flag}" HAVE_C_FLAG${flagname})
  if(HAVE_C_FLAG${flagname})
    add_c_flag(${flag})
  endif()
endmacro()

macro(add_check_cxx_flag flag)
  string(REGEX REPLACE "[-=]" "_" flagname ${flag})
  check_cxx_compiler_flag("${flag}" HAVE_CXX_FLAG${flagname})
  if(HAVE_CXX_FLAG${flagname})
    add_cxx_flag(${flag})
  endif()
endmacro()

# Check if C/CXX flag is supported and add to global list of C/CXX flags.
macro(add_check_c_cxx_flag flag)
  add_check_c_flag(${flag})
  add_check_cxx_flag(${flag})
endmacro()
