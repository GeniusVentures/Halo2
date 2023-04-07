//
// Created by Super Genius on 2/2/23.
//

#include "pch.h"

class xorwowTest : public testing::Test {
protected:
    PrivateKey sk;
    PublicKey pk;

public:
    void SetUp() override {
        PaillierCryptoSystem::generateKeys(pk, sk);
        GTEST_PRINT("pk: g: {#x} n: {#x} n2: {#x}\n",  pk.g, pk.n, pk.n2);
        GTEST_PRINT("sk: lambda: {#x} mu: {#x}\n", sk.lambda, sk.mu);
    }

};