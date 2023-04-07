//
// Created by Super Genius on 2/2/23.
//

#ifndef HALO2_PRIVATE_KEY_H
#define HALO2_PRIVATE_KEY_H

namespace halo2 {
    namespace crypto {

        /**
        * @brief A struct to represent a Paillier private key.
        */
        class PrivateKey {
        public:
            uint64_t lambda;  ///< The lambda component of the private key.
            uint64_t mu;      ///< The mu component of the private key.
            PrivateKey(uint64_t lambda = 0, uint64_t mu = 0) : lambda(lambda), mu(mu) {}
        };
    }
}

#endif //HALO2_PRIVATE_KEY_H
