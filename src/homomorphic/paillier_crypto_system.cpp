#include "../pch.h"

using namespace std;

namespace halo2 {
    namespace crypto {

        // Compute the power a^b modulo p using the binary exponentiation algorithm
        uint64_t PaillierCryptoSystem::fpow(uint64_t a, uint64_t b, uint64_t p) {
            uint64_t res = 1;
            while (b > 0) {
                if (b & 1) {
                    res = (res * a) % p;
                }
                a = (a * a) % p;
                b >>= 1;
            }
            return res;
        }

        // Compute the modular inverse of a modulo p using the extended Euclidean algorithm
        uint64_t PaillierCryptoSystem::inv(uint64_t a, uint64_t p) {
            uint64_t b = p;
            uint64_t x = 0;
            uint64_t y = 1;
            uint64_t t;
            while (a > 1) {
                uint64_t q = a / b;
                t = b;
                b = a % b;
                a = t;
                t = x;
                x = y - q * x;
                y = t;
            }
            return (y + p) % p;
        }

        // Encrypt a plaintext message using the Paillier public key
        Ciphertext PaillierCryptoSystem::encrypt(PublicKey &pk, uint64_t m) {
            uint64_t r = rand() % pk.n;
            Ciphertext ct;
            ct.x = fpow(pk.g, m, pk.n2);
            ct.y = fpow(r, pk.n, pk.n2);
            ct.x = (ct.x * ct.y) % pk.n2;
            return ct;
        }


        // Encrypt a plaintext message using the Paillier public key
        void PaillierCryptoSystem::encrypt(PublicKey &pk, uint64_t m, Ciphertext &out) {
            uint64_t r = rand() % pk.n;
            out.x = fpow(pk.g, m, pk.n2);
            out.y = fpow(r, pk.n, pk.n2);
            out.x = (out.x * out.y) % pk.n2;
        }

        // Encrypt a plaintext message using the Paillier public key
        void PaillierCryptoSystem::encrypt(PublicKey &pk, uint64_t m, uint64_t lambda, Ciphertext &out) {
            uint64_t r = rand() % pk.n;
            out.x = fpow(pk.g, m, pk.n2);
            out.y = fpow(r, pk.n, pk.n2);
            out.x = (out.x * out.y) % pk.n2;
            bootstrap(pk, lambda, out);
        }

        // Decrypt a Paillier ciphertext using the Paillier private key
        uint64_t PaillierCryptoSystem::decrypt(PublicKey &pk, PrivateKey &sk, Ciphertext &ct) {
            uint64_t x_inv = fpow(ct.x, sk.lambda, pk.n2);
            uint64_t x_inv_inv = inv(x_inv, pk.n2);
            uint64_t m = (x_inv_inv * ct.y) % pk.n2;
            m = fpow(m, sk.mu, pk.n2);
            m = (m - 1) / pk.n;
            return m;
        }

        // Decrypt a Paillier ciphertext using the Paillier private key
        void PaillierCryptoSystem::decrypt(PublicKey &pk, PrivateKey &sk, Ciphertext &ct, uint64_t &outValue) {
            uint64_t x_inv = fpow(ct.x, sk.lambda, pk.n2);
            uint64_t x_inv_inv = inv(x_inv, pk.n2);
            uint64_t m = (x_inv_inv * ct.y) % pk.n2;
            m = fpow(m, sk.mu, pk.n2);
            outValue = (m - 1) / pk.n;
        }

        // Perform bootstrapping on a Paillier ciphertext
        void PaillierCryptoSystem::bootstrap(PublicKey &pk, uint64_t lambda, Ciphertext &ct) {
            uint64_t x_inv = fpow(ct.x, lambda, pk.n2);
            uint64_t x_inv_inv = inv(x_inv, pk.n2);
            ct.x = (x_inv_inv * ct.x) % pk.n2;
            ct.y = (x_inv_inv * ct.y) % pk.n2;
        }

        /**
        * @brief Generates a new Paillier public and private key.
        *
        * @param pk A reference to the PublicKey object that will hold the generated public key.
        * @param sk A reference to the PrivateKey object that will hold the generated private key.
        */
        void PaillierCryptoSystem::generateKeys(PublicKey &pk, PrivateKey &sk)
        {
            RSA *rsa = RSA_new();
            BIGNUM *e = BN_new();
            BN_set_word(e, 65537);
            RSA_generate_key_ex(rsa, 2048, e, NULL);

            const BIGNUM *n, *d, *e_out;
            RSA_get0_key((const RSA *)rsa, &n, &e_out, &d);
            pk.n = BN_get_word(n);
            pk.n2 = pk.n * pk.n;
            pk.g = pk.n + 1;

            const BIGNUM *p, *q;
            RSA_get0_factors(rsa, &p, &q);
            uint64_t pw = BN_get_word(p);
            uint64_t qw = BN_get_word(q);
            sk.lambda = lcm(pw-1, qw-1);
            sk.mu = inv(modulo(pw * qw, pk.n), pk.n);

            BN_free(e);
            RSA_free(rsa);

        }







    } //namespace crypto
}  // namespace halo2
