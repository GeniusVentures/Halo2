//
// Created by Super Genius on 2/6/23.
//

#ifndef HALO2_TEST_PCH_H
#define HALO2_TEST_PCH_H

#include <gtest/gtest.h>
#include <fmt/color.h>
#include "../src/pch.h"

#define GTEST_PRINT(a, ...) fmt::print(fg(fmt::color::yellow), "[          ] "); \
    fmt::print(fg(fmt::color::yellow), a, __VA_ARGS__)

#endif //HALO2_TEST_PCH_H
