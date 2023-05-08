
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR} ./**.cpp ./**.c)
file(GLOB_RECURSE HEADERS RELATIVE ../ ./**.hpp ./**.h)

ADD_LIBRARY (Halo2 ${SOURCES})

set_target_properties(Halo2 PROPERTIES
        PUBLIC_HEADER "${HEADERS}"
        )

include_directories(Halo2 PRIVATE ${crypto3_DIR}/include)

target_link_libraries(Halo2
        PUBLIC OpenSSL::SSL
        ${Boost_LIBRARIES}
        spdlog::spdlog
        OpenSSL::Crypto
        )

