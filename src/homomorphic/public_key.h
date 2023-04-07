//
// Created by Super Genius on 2/2/23.
//

#ifndef HALO2_PUBLIC_KEY_H
#define HALO2_PUBLIC_KEY_H

namespace halo2 {
    namespace crypto {

        /**
        * @brief A class to represent a Paillier public key.
        */
        class PublicKey {
        public:
            uint64_t n;       ///< the n component of the public key
            uint64_t g;       ///< the g component of the public key
            uint64_t n2;      ///< pre-calculated n*n
            PublicKey(uint64_t n = 0, uint64_t g = 0) : n(n), g(g), n2(n * n) {}
        };
    }
}
#endif //HALO2_PUBLIC_KEY_H
