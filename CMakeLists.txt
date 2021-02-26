CMAKE_MINIMUM_REQUIRED(VERSION 3.2 FATAL_ERROR)

PROJECT(clog
    LANGUAGES C
    DESCRIPTION "Log Macros and Output Management in C"
    VERSION 1.0.0
)

###
### Install
###

ADD_LIBRARY(clog INTERFACE)

TARGET_INCLUDE_DIRECTORIES(
    clog INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>
)

INSTALL(
    TARGETS clog
    EXPORT clogTargets
    INCLUDES DESTINATION include
)

INCLUDE(CMakePackageConfigHelpers)

WRITE_BASIC_PACKAGE_VERSION_FILE(
    ${CMAKE_BINARY_DIR}/clogConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

INSTALL(
    EXPORT clogTargets
    NAMESPACE clog::
    DESTINATION lib/cmake/clog
)

CONFIGURE_PACKAGE_CONFIG_FILE(
    ${CMAKE_SOURCE_DIR}/cmake/clogConfig.cmake.in
    ${CMAKE_BINARY_DIR}/clogConfig.cmake
    INSTALL_DESTINATION lib/cmake/clog
)

INSTALL(
    FILES 
        ${CMAKE_BINARY_DIR}/clogConfigVersion.cmake
        ${CMAKE_BINARY_DIR}/clogConfig.cmake
    DESTINATION lib/cmake/clog
)

INCLUDE(FindPkgConfig)

IF(PKG_CONFIG_FOUND)
    CONFIGURE_FILE(
        ${CMAKE_SOURCE_DIR}/cmake/clog.pc.in
        ${CMAKE_BINARY_DIR}/clog.pc
        @ONLY
    )

    INSTALL(
        FILES ${CMAKE_BINARY_DIR}/clog.pc
        DESTINATION share/pkgconfig
    )
ENDIF()

INSTALL(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include
)
