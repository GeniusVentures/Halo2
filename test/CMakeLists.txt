
# --------------------------------------------------------
# Set config of GTest
set(GTest_DIR "${_THIRDPARTY_BUILD_DIR}/GTest/lib/cmake/GTest")
set(GTest_INCLUDE_DIR "${_THIRDPARTY_BUILD_DIR}/GTest/include")
find_package(GTest CONFIG REQUIRED)
include_directories(${GTest_INCLUDE_DIR})

addtest(xorwow_test
        xorwow_test.cpp
        )

target_link_libraries(xorwow_test
        Halo2
        )

addtest(paillier_test
        paillier_test.cpp
        )

target_link_libraries(paillier_test
        Halo2
        )