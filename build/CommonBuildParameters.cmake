
# BOOST VERSION TO USE
set(BOOST_MAJOR_VERSION "1" CACHE STRING "Boost Major Version")
set(BOOST_MINOR_VERSION "80" CACHE STRING "Boost Minor Version")
set(BOOST_PATCH_VERSION "0" CACHE STRING "Boost Patch Version")
# convenience settings
set(BOOST_VERSION "${BOOST_MAJOR_VERSION}.${BOOST_MINOR_VERSION}.${BOOST_PATCH_VERSION}")
set(BOOST_VERSION_3U "${BOOST_MAJOR_VERSION}_${BOOST_MINOR_VERSION}_${BOOST_PATCH_VERSION}")
set(BOOST_VERSION_2U "${BOOST_MAJOR_VERSION}_${BOOST_MINOR_VERSION}")

include(${PROJECT_ROOT}/cmake/functions.cmake)
find_package(OpenSSL REQUIRED)

# --------------------------------------------------------
# Set config of openssl project
set(OPENSSL_DIR "${_THIRDPARTY_BUILD_DIR}/openssl/build/${CMAKE_SYSTEM_NAME}${ABI_SUBFOLDER_NAME}")
set(OPENSSL_USE_STATIC_LIBS ON)
set(OPENSSL_MSVC_STATIC_RT ON)
set(OPENSSL_ROOT_DIR "${OPENSSL_DIR}")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_DIR}/include")
set(OPENSSL_LIBRARIES "${OPENSSL_DIR}/lib")
set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_LIBRARIES}/libcrypto${CMAKE_STATIC_LIBRARY_SUFFIX})
set(OPENSSL_SSL_LIBRARY ${OPENSSL_LIBRARIES}/libssl${CMAKE_STATIC_LIBRARY_SUFFIX})

include_directories(${OPENSSL_INCLUDE_DIR})

# --------------------------------------------------------
# Set config of fmt
set(fmt_DIR "${_THIRDPARTY_BUILD_DIR}/fmt/lib/cmake/fmt")
set(fmt_INCLUDE_DIR "${_THIRDPARTY_BUILD_DIR}/fmt/include")
find_package(fmt CONFIG REQUIRED)
include_directories(${fmt_INCLUDE_DIR})

# --------------------------------------------------------
# Set config of spdlog v1.4.2
set(spdlog_DIR "${_THIRDPARTY_BUILD_DIR}/spdlog/lib/cmake/spdlog")
set(spdlog_INCLUDE_DIR "${_THIRDPARTY_BUILD_DIR}/spdlog/include")
find_package(spdlog CONFIG REQUIRED)
include_directories(${spdlog_INCLUDE_DIR})
add_compile_definitions("SPDLOG_FMT_EXTERNAL")

# --------------------------------------------------------
# Set config of kompute
include_directories("${THIRDPARTY_DIR}/kompute/src/include")
link_libraries("${_THIRDPARTY_BUILD_DIR}/kompute/src/kompute-build/src/libkompute.a")

# --------------------------------------------------------
# Set config of vulkan headers
include_directories("${THIRDPARTY_DIR}/Vulkan-Headers/include")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------
include_directories(
        ${PROJECT_ROOT}/src
)
add_subdirectory(${PROJECT_ROOT}/src ${CMAKE_BINARY_DIR}/src)

ADD_DEFINITIONS(-D_HAS_AUTO_PTR_ETC=1)

print("CMAKE_HOST_SYSTEM_NAME: ${CMAKE_HOST_SYSTEM_NAME}")
print("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
print("C flags: ${CMAKE_C_FLAGS}")
print("CXX flags: ${CMAKE_CXX_FLAGS}")
print("C Debug flags: ${CMAKE_C_FLAGS_DEBUG}")
print("CXX Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
print("C Release flags: ${CMAKE_C_FLAGS_RELEASE}")
print("CXX Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
print("CXX Defines: ${CXX_DEFINES}")
print("CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
print("CXX_STANDARD: ${CXX_STANDARD}")

if (TESTING)
  enable_testing()
  add_subdirectory(${PROJECT_ROOT}/test ${CMAKE_BINARY_DIR}/test)
endif ()

if (BUILD_EXAMPLES)
    add_subdirectory(${PROJECT_ROOT}/example ${CMAKE_BINARY_DIR}/example)
endif ()

install(
  EXPORT Halo2Targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Halo2
  NAMESPACE halo2::
)

# generate the config file that is includes the exports
configure_package_config_file(${PROJECT_ROOT}/cmake/config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/Halo2Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Halo2
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# generate the version file for the config file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/Halo2ConfigVersion.cmake"
  VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}"
  COMPATIBILITY AnyNewerVersion
)

### install header files ###
install_hfile(${PROJECT_ROOT}/include)

# install the configuration file
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/Halo2Config.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Halo2
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/Halo2ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Halo2
)
