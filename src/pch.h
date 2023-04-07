//
// Created by Super Genius on 2/2/23.
//

#ifndef HALO2_PCH_H
#define HALO2_PCH_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <cmath>
#include <variant>

using namespace std;

// openssl
#include <openssl/pem.h>
#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>


#include "homomorphic/private_key.h"
#include "homomorphic/public_key.h"
#include "homomorphic/cipher_text.h"
#include "homomorphic/paillier_crypto_system.h"

#include "logger.hpp"
#include "xorwow.h"

#endif //HALO2_PCH_H
