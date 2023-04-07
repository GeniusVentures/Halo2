//
// Created by Super Genius on 1/6/23.
//

#ifndef HALO2_PAILLIER_CRYPTO_SYSTEM_H
#define HALO2_PAILLIER_CRYPTO_SYSTEM_H

namespace halo2 {
    namespace crypto {

        /**
        * @brief The PaillierCryptoSystem class, which provides methods for performing
        *        homomorphic encryption and decryption using the Paillier cryptosystem.
        */
        class PaillierCryptoSystem {
        public:
            /**
            * @brief Computes the power a^b modulo p using the binary exponentiation algorithm.
            *
            * @param a The base of the exponentiation.
            * @param b The exponent of the exponentiation.
            * @param p The modulus.
            *
            * @return The result of the exponentiation, a^b mod p.
            */
            static uint64_t fpow(uint64_t a, uint64_t b, uint64_t p);

            /**
            * @brief Computes the modular inverse of a modulo p using the extended Euclidean algorithm.
            *
            * @param a The number whose modular inverse is to be computed.
            * @param p The modulus.
            *
            * @return The modular inverse of a mod p, if it exists.
            */
            static uint64_t inv(uint64_t a, uint64_t p);

            /**
            * @brief Encrypts a plaintext message using the Paillier public key.
            *
            * @param pk The Paillier public key to use for encryption.
            * @param m  The plaintext message to encrypt.
            *
            * @return The Paillier ciphertext resulting from encrypting the plaintext message.
            */
            static Ciphertext encrypt(PublicKey &pk, uint64_t m);


            /**
            * @brief Encrypts a plaintext message using the Paillier public key.
            *
            * @param pk The Paillier public key to use for encryption.
            * @param m  The plaintext message to encrypt.
            * @param out The CipherText reference to fill in
            *
            */
            static void encrypt(PublicKey &pk, uint64_t m, Ciphertext &out);

            /**
            * @brief Encrypts a plaintext message using the Paillier public key and performs bootstrapping.
            *
            * @param pk The Paillier public key to use for encryption.
            * @param lambda The Paillier private key lambda member
            * @param m  The plaintext message to encrypt.
            * @param out The CipherText reference to fill in
            *
            */
            static void encrypt(PublicKey &pk, uint64_t m, uint64_t lambda, Ciphertext &out);

            /**
            * @brief Decrypts a Paillier ciphertext using the Paillier keys.
            *
            * @param pk The Paillier public key to use for decryption.
            * @param sk The Paillier private key to use for decryption.
            * @param ct The Paillier ciphertext to decrypt.
            *
            * @return The plaintext message resulting from decrypting the ciphertext.
            */
            static uint64_t decrypt(PublicKey &pk, PrivateKey &sk, Ciphertext &ct);

            /**
            * @brief Decrypts a Paillier ciphertext using the Paillier keys.
            *
            * @param pk The Paillier public key to use for decryption.
            * @param sk The Paillier private key to use for decryption.
            * @param ct The Paillier ciphertext to decrypt.
            * @param outValue a reference to the output to store the plaintext message resulting from decrypting the ciphertext.
            */
            static void decrypt(PublicKey &pk, PrivateKey &sk, Ciphertext &ct, uint64_t &outValue);

            /**
            * @brief Performs bootstrapping on a Paillier ciphertext.
            *
            * @param pk The Paillier public key.
            * @param sk The Paillier lambda of the private key
            * @param ct The Paillier ciphertext to perform bootstrapping on.
            *
            */
            static void bootstrap(PublicKey &pk, uint64_t lambda, Ciphertext &ct);

            /**
             * @brief Generates a public key and private key for the Paillier cryptosystem.
             *
             * @param pubKey A reference to a PublicKey struct to store the generated public key.
             * @param privKey A reference to a PrivateKey struct to store the generated private key.
             */
            static void generateKeys(PublicKey &pubKey, PrivateKey &privKey);

            /// helper functions
            static inline uint64_t modulo(uint64_t a, uint64_t b) {
                return a % b;
            }

            static inline uint64_t gcd(uint64_t a, uint64_t b) {
                return b == 0 ? a : gcd(b, modulo(a, b));
            }

            static inline uint64_t lcm(uint64_t a, uint64_t b) {
                return a * b / gcd(a, b);
            }

        };

    } // namespace crypto
}  // namespace halo2

#endif  // HALO2_PAILLIER_CRYPTO_SYSTEM_H

