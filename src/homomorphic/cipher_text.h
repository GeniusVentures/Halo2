//
// Created by Super Genius on 2/2/23.
//

#ifndef HALO2_CIPHERTEXT_H
#define HALO2_CIPHERTEXT_H

namespace halo2 {
    namespace crypto {

/**
* @brief A class to represent a Paillier ciphertext.
*/
        class Ciphertext {
        public:
            uint64_t x;  ///< The x component of the ciphertext.
            uint64_t y;  ///< The y component of the ciphertext.
            Ciphertext(uint64_t x = 0, uint64_t y = 0) : x(x), y(y) {}

            inline Ciphertext &operator^=(const Ciphertext &other) {
                x = x ^ other.x;
                y = y ^ other.y;
                return *this;
            }

            inline Ciphertext &operator>>=(const int shift) {
                x = x >> shift;
                y = y >> shift;
                return *this;
            }

            inline Ciphertext &operator%=(const Ciphertext &other) {
                x = x % other.x;
                y = y % other.y;
                return *this;
            }

            inline Ciphertext &operator+=(const Ciphertext &other) {
                x = x + other.x;
                y = y + other.y;
                return *this;
            }

            inline Ciphertext &operator-=(const Ciphertext &other) {
                x = x - other.x;
                y = y - other.y;
                return *this;
            }

            inline Ciphertext &operator*=(const Ciphertext &other) {
                x = x * other.x;
                y = y * other.y;
                return *this;
            }

            inline Ciphertext &operator/=(const Ciphertext &other) {
                x = x / other.x;
                y = y / other.y;
                return *this;
            }

            Ciphertext operator+(const Ciphertext &other) const {
                return Ciphertext{x + other.x, y + other.y};
            }

            Ciphertext operator-(const Ciphertext &other) const {
                return Ciphertext{x - other.x, y - other.y};
            }

            Ciphertext operator*(const Ciphertext &other) const {
                return Ciphertext{x * other.x, y * other.y};
            }

            Ciphertext operator/(const Ciphertext &other) const {
                return Ciphertext{x / other.x, y / other.y};
            }

            Ciphertext operator%(const Ciphertext &other) const {
                return Ciphertext{x % other.x, y % other.y};
            }

            Ciphertext operator^(const Ciphertext &other) const {
                return Ciphertext{x ^ other.x, y ^ other.y};
            }

            Ciphertext operator>>(const uint32_t shift) const {
                return Ciphertext{x >> shift, y >> shift};
            }

            Ciphertext operator<<(const uint32_t shift) const {
                return Ciphertext{x << shift, y << shift};
            }


        };
    }
}
#endif //HALO2_CIPHERTEXT_H
