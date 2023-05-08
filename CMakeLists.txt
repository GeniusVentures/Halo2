cmake_minimum_required(VERSION 3.2)

project(Halo2 VERSION 0.1)

set(CONFIG_NAME "Halo2Config")
set(CONFIG_DESTINATION_DIR "${CMAKE_INSTALL_PREFIX}/cmake/${PROJECT_NAME}")

include(GNUInstallDirs)

include(cmake/functions.cmake)

find_package(OpenSSL REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# BOOST library
find_package(Boost REQUIRED COMPONENTS random filesystem date_time regex)

add_subdirectory(src)

option(TESTING "Build tests" OFF)

if (TESTING)
    find_package(GTest CONFIG REQUIRED)
endif ()

# Tests building
if (TESTING)
    include(cmake/test.cmake)
    enable_testing()
    add_subdirectory(test)
endif ()

install(DIRECTORY "${CMAKE_SOURCE_DIR}/src/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" FILES_MATCHING  PATTERN "*.hpp")

install(TARGETS Halo2
        EXPORT Halo2Targets DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        EXPORT ${CONFIG_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_PREFIX}
)

include(CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/Halo2Config.cmake"
  INSTALL_DESTINATION "lib/cmake/examples"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# generate the version file for the config file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/Halo2ConfigVersion.cmake"
  VERSION "${Halo2_VERSION_MAJOR}.${Halo2_VERSION_MINOR}"
  COMPATIBILITY AnyNewerVersion
)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/src/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" FILES_MATCHING  PATTERN "*.h*")

# install the configuration file
install(
        EXPORT ${CONFIG_NAME}
        NAMESPACE Halo2::
        DESTINATION ${CONFIG_DESTINATION_DIR}
)