#ifndef SG_HALO2_POSEIDON_HPP
#define SG_HALO2_POSEIDON_HPP

#include "pow5.hpp"
#include "primitives.hpp"

namespace halo2::gadgets {

    using namespace primitives;
    using namespace poseidon;

    // Export the Pow5Chip, Pow5Config and StateWord classes
    using Pow5Chip = pow5::Pow5Chip<Field>;
    using Pow5Config = pow5::Pow5Config<Field>;
    using StateWord = pow5::StateWord<Field>;

    using Absorbing = primitives::Absorbing<Field, PoseidonArity>;
    using ConstantLength = primitives::ConstantLength<Field, PoseidonArity>;
    using Domain = primitives::Domain<Field, PoseidonArity>;
    using Spec = primitives::Spec<Field, PoseidonArity>;
    using SpongeMode = primitives::SpongeMode<Field, PoseidonArity>;
    using Squeezing = primitives::Squeezing<Field, PoseidonArity>;
    using State = primitives::State<Field, PoseidonArity>;


}

#endif // 
