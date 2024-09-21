cmake_minimum_required(VERSION 3.16)

option(ENABLE_WIX "Enable generating installer with WiX" OFF)
option(SUPPRESS_WIX_VALIDATION "Suppress validation in WiX" OFF)
option(INSTALL_QT "Install QT dlls when ENABLE_WIX is off (windows only)" OFF)

set(OCTCALC_VERSION_MAJOR 0)
set(OCTCALC_VERSION_MINOR 4)
set(OCTCALC_VERSION_PATCHLEVEL 1)
set(OCTCALC_VERSION ${OCTCALC_VERSION_MAJOR}.${OCTCALC_VERSION_MINOR}.${OCTCALC_VERSION_PATCHLEVEL})
set(OCTCALC_SOVERSION ${OCTCALC_VERSION_MAJOR})

message(STATUS "Configuring OctCalc ${OCTCALC_VERSION_MAJOR}.${OCTCALC_VERSION_MINOR}.${OCTCALC_VERSION_PATCHLEVEL}")

project(octcalc LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED HINTS "c:/opt/qt6")
find_package(Qt6 COMPONENTS Test REQUIRED HINTS "c:/opt/qt6")

if(WIN32 OR NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" LC_CMAKE_BUILD_TYPE)

enable_testing()

# Include submodules

set(SUBMODULE_INSTALL_DIR "${CMAKE_BINARY_DIR}/submodules")

include(ExternalProject)
include(FindPkgConfig)

if (NOT EXISTS "${PROJECT_SOURCE_DIR}/submodules")
  file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/submodules")
endif()

# TLFloat

set(TLFLOAT_MINIMUM_VERSION 1.8.0)
set(TLFLOAT_GIT_TAG "4bd539839ccf4d442cf4bf3caf62564d63a7842a")

#

set(TLFLOAT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/submodules/tlfloat")
set(TLFLOAT_INSTALL_DIR "${SUBMODULE_INSTALL_DIR}/tlfloat")

set(TLFLOAT_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${TLFLOAT_INSTALL_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILD_TESTS=False)

if (CMAKE_C_COMPILER)
  list(APPEND TLFLOAT_CMAKE_ARGS -DCMAKE_C_COMPILER:PATH=${CMAKE_C_COMPILER})
endif()

if (CMAKE_CXX_COMPILER)
  list(APPEND TLFLOAT_CMAKE_ARGS -DCMAKE_CXX_COMPILER:PATH=${CMAKE_CXX_COMPILER})
endif()

if (CMAKE_TOOLCHAIN_FILE)
  list(APPEND TLFLOAT_CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif()

if (EXISTS "${TLFLOAT_SOURCE_DIR}/CMakeLists.txt")
  # If the source code of tlfloat is already downloaded, use it
  ExternalProject_Add(ext_tlfloat
    SOURCE_DIR "${TLFLOAT_SOURCE_DIR}"
    CMAKE_ARGS ${TLFLOAT_CMAKE_ARGS}
    )
  include_directories(BEFORE "${TLFLOAT_INSTALL_DIR}/include")
  link_directories("${TLFLOAT_INSTALL_DIR}/lib")
else()
  pkg_search_module(TLFLOAT tlfloat)

  if (TLFLOAT_FOUND AND TLFLOAT_VERSION VERSION_GREATER_EQUAL TLFLOAT_MINIMUM_VERSION)
    # If tlfloat is installed on the system
    add_custom_target(ext_tlfloat ALL)
    include_directories(BEFORE "${TLFLOAT_INCLUDE_DIRS}")
    link_directories("${TLFLOAT_LIBDIR}")
    message(STATUS "Found installed TLFloat " ${TLFLOAT_VERSION})
  else()
    # Otherwise, download the source code
    find_package(Git REQUIRED)
    ExternalProject_Add(ext_tlfloat
      GIT_REPOSITORY https://github.com/shibatch/tlfloat
      GIT_TAG "${TLFLOAT_GIT_TAG}"
      SOURCE_DIR "${TLFLOAT_SOURCE_DIR}"
      CMAKE_ARGS ${TLFLOAT_CMAKE_ARGS}
      )

    include_directories(BEFORE "${TLFLOAT_INSTALL_DIR}/include")
    link_directories("${TLFLOAT_INSTALL_DIR}/lib")
  endif()
endif()


# Setup WIX

if (WIN32 AND ENABLE_WIX)
  set(CPACK_PACKAGE_NAME "OctCalc")
  set(CPACK_PACKAGE_VENDOR "Naoki Shibata")
  set(CPACK_PACKAGE_DESCRIPTION "Octuple precision calculator")
  set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/shibatch/octcalc")
  set(CPACK_WIX_PROPERTY_ARPCOMMENTS "OctCalc")
  set(CPACK_WIX_PROPERTY_ARPHELPLINK "https://github.com/shibatch/octcalc")
  set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "https://github.com/shibatch/octcalc")

  set(CPACK_WIX_UPGRADE_GUID "5f89bb9b-edb8-4ca3-9e6c-27df3858054c")
  set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.txt)
  set(CPACK_PACKAGE_NAME "OctCalc")
  set(CPACK_PACKAGE_VERSION "${OCTCALC_VERSION}")
  set(CPACK_WIX_PROGRAM_MENU_FOLDER "OctCalc")

  SET(CMAKE_INSTALL_PREFIX "C:/Program Files/OctCalc" CACHE PATH "Install path" FORCE)
  SET(CPACK_SET_DESTDIR FALSE)
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "OctCalc")
  SET(CPACK_PACKAGING_INSTALL_PREFIX "")

  if (SUPPRESS_WIX_VALIDATION)
    list(APPEND CPACK_WIX_LIGHT_EXTRA_FLAGS "-sval")
  endif()

  #

  find_program(WINDEPLOYQT_EXECUTABLE windeployqt)

  #

  set(INSTALL_PREFIX "")
  set(CPACK_GENERATOR WIX)
  include(CPack)
else()
  set(INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/")
endif()

set(INSTALL_BINDIR "${INSTALL_PREFIX}bin")
set(INSTALL_DATADIR "${INSTALL_PREFIX}share/octcalc")

add_compile_definitions(OCTCALC_VERSION_MAJOR=${OCTCALC_VERSION_MAJOR})
add_compile_definitions(OCTCALC_VERSION_MINOR=${OCTCALC_VERSION_MINOR})
add_compile_definitions(OCTCALC_VERSION_PATCHLEVEL=${OCTCALC_VERSION_PATCHLEVEL})

add_subdirectory("src")

message(STATUS "CMAKE_BUILD_TYPE : " ${CMAKE_BUILD_TYPE})
