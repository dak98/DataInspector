###############################################################################
#    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    #
#                                                                              #
#              This software is distributed under the terms of the             #
#         GNU Lesser General Public Licence version 3 (LGPL) version 3,        #
#                  copied verbatim in the file "LICENSE"                       #
################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.7)
CMAKE_POLICY(SET CMP0028 NEW)

PROJECT(InspectorPlugin LANGUAGES CXX)
SET(CMAKE_CXX_STANDARD 17)

SET(PLUGIN FairMQPlugin_plugin)
SET(VERSION_MAJOR 1)
SET(VERSION_MINOR 0)
SET(VERSION_PATCH 0)
SET(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

IF (MODULES_DIR)
  SET(CMAKE_PREFIX_PATH
    "${MODULES_DIR}/FairMQ/latest"
    "${MODULES_DIR}/FairLogger/latest"
    "${MODULES_DIR}/fmt/latest"
    "${MODULES_DIR}/boost/latest")
  SET(ENV{PKG_CONFIG_PATH} "${MODULES_DIR}/ZeroMQ/latest/lib/pkgconfig")
ELSE()
  MESSAGE(FATAL_ERROR "Specify MODULES_DIR to the root of modules installation")
ENDIF()

# Dependencies
FIND_PACKAGE(FairMQ REQUIRED)
FIND_PACKAGE(FairLogger REQUIRED)
FIND_PACKAGE(fmt REQUIRED)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system)
FIND_PACKAGE(PkgConfig REQUIRED) # Needed for ZeroMQ
PKG_CHECK_MODULES(ZeroMQ REQUIRED IMPORTED_TARGET libzmq)

# Paths
IF (FairMQ_FOUND)
  SET(FairMQ_INCLUDE_DIR ${FairMQ_INCDIR} ${FairMQ_INCDIR}/fairmq)
  SET(FairMQ_LIBRARY_DIR ${FairMQ_LIBDIR})
ENDIF()

IF (FairLogger_FOUND)
  SET(FairLogger_INCLUDE_DIR ${FairLogger_INCDIR})
ENDIF()

IF (Boost_FOUND)
  SET(Boost_INCLUDE_DIR ${Boost_INCLUDE_DIRS})
  SET(Boost_LIBRARY_DIR ${Boost_SYSTEM_LIBRARY})
ENDIF()

IF (ZeroMQ_FOUND)
  SET(ZeroMQ_INCLUDE_DIR ${ZeroMQ_INCLUDE_DIRS})
  SET(ZeroMQ_LIBRARY_DIR ${ZeroMQ_LINK_LIBRARIES})
ENDIF()

# Include directories and libraries
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${FairMQ_INCLUDE_DIR}
  ${FairLogger_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${ZeroMQ_INCLUDE_DIR})
LINK_DIRECTORIES(
  ${FairMQ_LIBRARY_DIRS})
LINK_LIBRARIES(
  ${Boost_SYSTEM_LIBRARY}
  ${ZeroMQ_LIBRARY_DIR})

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/InspectorPlugin.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/InspectorPlugin.h)

# Plugin configuration
ADD_LIBRARY(${PLUGIN} SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/InspectorPlugin.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/InspectorPlugin.h)

TARGET_LINK_LIBRARIES(${PLUGIN}
  PUBLIC fmt::fmt-header-only)
TARGET_INCLUDE_DIRECTORIES(${PLUGIN}
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

SET_TARGET_PROPERTIES(${PLUGIN} PROPERTIES CXX_VISIBILITY_PRESET hidden)
SET_TARGET_PROPERTIES(${PLUGIN} PROPERTIES VERSION ${VERSION})
