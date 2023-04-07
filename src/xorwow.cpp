//
// Created by Super Genius on 1/31/23.
//

#include "pch.h"

using namespace halo2::crypto;

xorwow::xorwow(uint64_t seed, halo2::crypto::PublicKey *pk) {
    if (pk) {
        _encrypted = true;
        _pk.g = pk->g;
        _pk.n = pk->n;
        _pk.n2 = _pk.n * _pk.n;
        auto state = get<XORWOW_STATE_ENCRYPTED>(_state);
        PaillierCryptoSystem::encrypt(_pk, XOR_ADD_VALUE, _xorAddValue);
        PaillierCryptoSystem::encrypt(_pk,UINT64_MAX + 1, _maxIntValue);
        PaillierCryptoSystem::encrypt(_pk, seed & UINT32_MAX, state.x[0]);
        PaillierCryptoSystem::encrypt(_pk, seed >> 32, state.x[1]);
        PaillierCryptoSystem::encrypt(_pk, (seed & UINT32_MAX) ^ 0xdeadbeef, state.x[2]);
        PaillierCryptoSystem::encrypt(_pk, (seed >> 32) ^ 0xdeadbeef, state.x[3]);
        PaillierCryptoSystem::encrypt(_pk, 0xdeadc0de, state.x[4]);
        PaillierCryptoSystem::encrypt(_pk, 0, state.counter);
    } else {
        auto state = get<XORWOW_STATE>(_state);
        state.x[0] = seed & UINT32_MAX;
        state.x[1] = seed >> 32;
        state.x[2] = state.x[0] ^ 0xdeadbeef;
        state.x[3] = state.x[1] ^ 0xdeadbeef;
        state.x[4] = 0xdeadc0de;
        state.counter = 0;
    }
}


XRAND_VALUE xorwow::rand() {
    XRAND_VALUE v;
    if (_encrypted) {
        auto state = get<XORWOW_STATE_ENCRYPTED>(_state);
        Ciphertext t = state.x[4];
        const Ciphertext s = state.x[0];
        state.x[4] = state.x[3];
        state.x[3] = state.x[2];
        state.x[2] = state.x[1];
        state.x[1] = s;

        t ^= (t >> 2);
        t ^= (t << 1);
        t ^= (s ^ (s << 4));

        state.x[0] = t;
        state.counter = (state.counter + _xorAddValue);
        v = (t + state.counter);

    } else {
        auto state = get<XORWOW_STATE>(_state);
        std::uint64_t t = state.x[4];
        const std::uint64_t s = state.x[0];
        state.x[4] = state.x[3];
        state.x[3] = state.x[2];
        state.x[2] = state.x[1];
        state.x[1] = s;

        t ^= (t >> 2);
        t ^= (t << 1);
        t ^= (s ^ (s << 4));

        state.x[0] = t;
        state.counter = (state.counter + 362437);
        v = (t + state.counter);
    }

    return v;
}
