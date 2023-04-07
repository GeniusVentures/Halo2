//
// Created by Super Genius on 2/3/23.
//

#include "pch.h"
#define NUM_VALUES 4

class PaillierTest : public testing::Test {
protected:
    PrivateKey sk;
    PublicKey pk;
    Ciphertext encCipherTest[NUM_VALUES];
    static constexpr uint64_t plainText[NUM_VALUES] = { 0xdeadbeef, 0xdeadc0de, 0xbeeffead, 0xc0defeed };

public:
    void SetUp() override {
        PaillierCryptoSystem::generateKeys(pk, sk);
        GTEST_PRINT("pk: g: {:#x} n: {:#x} n2: {:#x}\n",  pk.g, pk.n, pk.n2);
        GTEST_PRINT("sk: lambda: {:#x} mu: {:#x}\n", sk.lambda, sk.mu);
    }

};

TEST_F(PaillierTest, TestEncryptDecrypt) {

    for (u_int i=0; i < NUM_VALUES; i++) {
         PaillierCryptoSystem::encrypt(pk, plainText[i], sk.lambda, encCipherTest[i]);
         GTEST_PRINT("Ciphertext: x: {:#x} y: {:#x}\n", encCipherTest[i].x, encCipherTest[i].y);
    }

    for (u_int i=0; i < NUM_VALUES; i++) {
        uint64_t decText[NUM_VALUES];
        PaillierCryptoSystem::decrypt(pk, sk, encCipherTest[i], decText[i]);

        EXPECT_TRUE(decText[i] == plainText[i]) << "Decrypted value of index " << i << "doesn't match";
    }


}