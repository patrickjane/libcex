cmake_minimum_required(VERSION 3.2.0)
project(cex)

set(CEX_VERSION_MAJOR 1)
set(CEX_VERSION_MINOR 0)
set(CEX_VERSION_PATCH 0)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

# add more cmake rules (to find libevent & libevhtp & libz)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find mandatory libraries libevent + libevhtp

find_package(LibEvent REQUIRED)
list(APPEND LIBCEX_EXTERNAL_LIBS ${LIBEVENT_LIBRARIES})
list(APPEND LIBCEX_EXTERNAL_INCLUDES ${LIBEVHTP_INCLUDE_DIRS})
list(APPEND package_deps LibEvent)

find_package(LibEvhtp REQUIRED)
list(APPEND LIBCEX_EXTERNAL_LIBS ${LIBEVHTP_LIBRARIES})
list(APPEND LIBCEX_EXTERNAL_INCLUDES ${LIBEVENT_INCLUDE_DIRS})
list(APPEND package_deps LibEvhtp)

# Find optional libraries openssl + zlib

if(NOT CEX_DISABLE_SSL)
    find_package(OpenSSL)
    if(OPENSSL_FOUND)
        set(CEX_WITH_SSL "true")
        add_definitions(-DCEX_WITH_SSL)
        list(APPEND LIBCEX_EXTERNAL_LIBS OpenSSL::SSL OpenSSL::Crypto)
        list(APPEND package_deps OpenSSL)
    endif()
endif()

if(NOT CEX_DISABLE_Z)
   find_package(LibZ)
   if(LIBZ_FOUND)
      set(CEX_WITH_ZLIB "true")
      add_definitions(-DCEX_WITH_ZLIB)
      list(APPEND LIBCEX_EXTERNAL_INCLUDES ${LIBZ_INCLUDE_DIRS})
      list(APPEND LIBCEX_EXTERNAL_LIBS ${LIBZ_LIBRARIES})
      list(APPEND package_deps LibZ)
   endif()
endif()


include_directories(${LIBCEX_EXTERNAL_INCLUDES})

# configure a header file to pass some of the CMake settings to the source code

configure_file (
  "${PROJECT_SOURCE_DIR}/CexConfig.h.in"
  "${PROJECT_SOURCE_DIR}/include/cex/cex_config.h"
  )

# add project headers

include_directories(include)
include_directories(include/cex)

# add project sources

file(GLOB SOURCES "src/*.cc")

# generate the library

add_library(cex SHARED ${SOURCES})

# compiler options & dependencies

target_compile_features(cex PRIVATE cxx_range_for)

if (APPLE)
   target_link_libraries(cex PUBLIC ${LIBCEX_EXTERNAL_LIBS})
endif()

# set locations of lib installation (shall be relative to CMAKE_INSTALL_PREFIX, which is /usr/local in most cases)
# will create subdirectories /lib, /include and /include/cex

install(TARGETS cex DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include FILES_MATCHING PATTERN "*.h*")

# testcases will be in 'test' subfolder
# and be compiled in the ${BUILD_DIR}/test folder
# run with `make test` or `ctest`

enable_testing()
add_subdirectory(test)

