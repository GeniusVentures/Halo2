
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR} ./**.cpp ./**.c)
file(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_CURRENT_LIST_DIR} ./**.hpp ./**.h)

ADD_LIBRARY (Halo2 ${SOURCES})
set_target_properties(Halo2 PROPERTIES
        PUBLIC_HEADER "${HEADERS}"
        )

# find_package(OpenSSL REQUIRED)
target_link_libraries(Halo2
        PUBLIC OpenSSL::SSL
        spdlog::spdlog
        OpenSSL::Crypto
        )
halo2_install(Halo2)