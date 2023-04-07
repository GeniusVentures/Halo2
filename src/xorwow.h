//
// Created by Super Genius on 1/31/23.
//

#ifndef XORWOW_H
#define XORWOW_H

using namespace halo2::crypto;

/**
 * @brief XORWOW_STATE is the state of XORWOW generator
 */
struct XORWOW_STATE {
    std::uint64_t x[5];
    std::uint64_t counter;
};

/**
 * @brief XORWOW_STATE is the state of XORWOW generator
 */
struct XORWOW_STATE_ENCRYPTED {
    Ciphertext x[5];
    Ciphertext counter;
};

typedef variant<std:: uint64_t, Ciphertext> XRAND_VALUE;

#define XOR_ADD_VALUE 362437

/// Unencrypted state information
/// The state of the xorwow RNG encrypted

typedef variant<XORWOW_STATE, XORWOW_STATE_ENCRYPTED> STATE;

class xorwow {
  private:
    /// The public key used in paillier encryption operations
    PublicKey _pk;
    bool _encrypted = false;
    /// encrypted value of the xorAddValue
    Ciphertext _xorAddValue;
    /// encrypted value of the maxIntValue (std::UINT64_MAX +1 )
    Ciphertext _maxIntValue;
    STATE _state;

  public:

    /// initialize without encryption
    xorwow(uint64_t seed, PublicKey *pk = NULL);

    /**
     * @brief rand returns/generates next random number
     * @return the next random number, encrypted or unencrypted
     */
    XRAND_VALUE rand();

};


#endif // XORWOW_H
