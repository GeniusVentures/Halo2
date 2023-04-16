#ifndef SG_HALO2_POSEIDON_PRIMITIVES_HPP
#define SG_HALO2_POSEIDON_PRIMITIVES_HPP

#include <array>
#include <optional>
#include <algorithm>
#include <vector>
#include <iostream>
#include <cstdint>
#include <type_traits>

#include <group/ff.hpp>

using namespace group::ff;

namespace halo2::gadgets::poseidon {

    template <typename F, size_t T>
    using State = std::array<F, T>;
    
    template <typename F, size_t RATE>
    using SpongeRate = std::array<std::optional<F>, RATE>;
    
    template <typename F, size_t T>
    using Mds = std::array<std::array<F, T>, T>;

    template <typename F, size_t T, size_t RATE>
    struct Spec {
      static constexpr size_t full_rounds() {
        // The number of full rounds for this specification.
        // This must be an even number.
        return 0;
      }
    
      static constexpr size_t partial_rounds() {
        // The number of partial rounds for this specification.
        return 0;
      }
    
      static F sbox(const F& val) {
        // The S-box for this specification.
        return F(0);
      }
    
      static constexpr size_t secure_mds() {
        // Side-loaded index of the first correct and secure MDS that will be generated by
        // the reference implementation.
        // This is used by the default implementation of `constants()`. If you are
        // hard-coding the constants, you may leave this unimplemented.
        return 0;
      }
    
      static std::tuple<std::vector<std::array<F, T>>, Mds<F, T>, Mds<F, T>> constants() {
        // Generates `(round_constants, mds, mds^-1)` corresponding to this specification.
        return std::make_tuple(std::vector<std::array<F, T>>{}, Mds<F, T>{}, Mds<F, T>{});
      }
    };

    template <typename F, template <typename, std::size_t, std::size_t> class S, std::size_t T, std::size_t RATE>
    std::tuple<std::vector<std::array<F, T>>, Mds<F, T>, Mds<F, T>> generate_constants()
    {
        constexpr auto r_f = S<F, T, RATE>::full_rounds();
        constexpr auto r_p = S<F, T, RATE>::partial_rounds();
    
        Grain<F> grain(SboxType::Pow, T, r_f, r_p);
    
        std::vector<std::array<F, T>> round_constants(r_f + r_p);
        std::generate(round_constants.begin(), round_constants.end(), [&grain]() {
            std::array<F, T> rc_row;
            std::generate(rc_row.begin(), rc_row.end(), [&grain]() {
                return grain.next_field_element();
            });
            return rc_row;
        });
    
        const auto [mds, mds_inv] = generate_mds<F, T>(&grain, S<F, T, RATE>::secure_mds());
    
        return { round_constants, mds, mds_inv };
    }

    template <typename F, template <typename, std::size_t, std::size_t> class S, std::size_t T, std::size_t RATE>
    void permute(State<F, T>& state, const Mds<F, T>& mds, const std::vector<std::array<F, T>>& round_constants)
    {
        constexpr auto r_f = S<F, T, RATE>::full_rounds() / 2;
        constexpr auto r_p = S<F, T, RATE>::partial_rounds();
    
        const auto apply_mds = [&](State<F, T>& state) {
            State<F, T> new_state = {};
            // Matrix multiplication
            for (std::size_t i = 0; i < T; ++i) {
                for (std::size_t j = 0; j < T; ++j) {
                    new_state[i] += mds[i][j] * state[j];
                }
            }
            state = new_state;
        };
    
        const auto full_round = [&](State<F, T>& state, const std::array<F, T>& rcs) {
            for (std::size_t i = 0; i < T; ++i) {
                state[i] = S<F, T, RATE>::sbox(state[i] + rcs[i]);
            }
            apply_mds(state);
        };
    
        const auto part_round = [&](State<F, T>& state, const std::array<F, T>& rcs) {
            for (std::size_t i = 0; i < T; ++i) {
                state[i] += rcs[i];
            }
            // In a partial round, the S-box is only applied to the first state word.
            state[0] = S<F, T, RATE>::sbox(state[0]);
            apply_mds(state);
        };
    
        std::vector<std::function<void(State<F, T>&, const std::array<F, T>&)>> round_functions;
        round_functions.reserve(r_f + r_p);
        for (std::size_t i = 0; i < r_f; ++i) {
            round_functions.push_back(full_round);
        }
        for (std::size_t i = 0; i < r_p; ++i) {
            round_functions.push_back(part_round);
        }
        for (std::size_t i = 0; i < r_f; ++i) {
            round_functions.push_back(full_round);
        }
    
        auto round_constants_it = round_constants.cbegin();
        for (const auto& round_function : round_functions) {
            round_function(state, *round_constants_it++);
        }
    }

    template <typename F, typename S, size_t T, size_t RATE>
    Squeezing<F, RATE> poseidon_sponge(State<F, T>& state,
                                        const Absorbing<F, RATE>* input,
                                        const Mds<F, T>& mds_matrix,
                                        const std::array<std::array<F, T>, S::NUM_ROUNDS>& round_constants) {
        if (input != nullptr) {
            // This will only mutate the rate portion of the state.
            for (size_t i = 0; i < RATE; ++i) {
                state[i] += input->get_word(i);
            }
        }
    
        permute<F, S, T, RATE>(state, mds_matrix, round_constants);
    
        std::array<std::optional<F>, RATE> output;
        for (size_t i = 0; i < RATE; ++i) {
            output[i] = state[i];
        }
        return Squeezing<F, RATE>(output);
    }

    // Forward declarations
    template <typename F, size_t RATE> struct SpongeRate;
    template <typename F, size_t RATE> struct Absorbing;
    template <typename F, size_t RATE> struct Squeezing;
    
    namespace private {
        // Empty trait to ensure that `Absorbing` and `Squeezing` are the only possible
        // instantiations of `SpongeMode`.
        template <typename F, size_t RATE> struct SealedSpongeMode {};
        template <typename F, size_t RATE> struct SealedSpongeMode<Absorbing<F, RATE>> {};
        template <typename F, size_t RATE> struct SealedSpongeMode<Squeezing<F, RATE>> {};
    }  // namespace private
    
    // The state of the `Sponge`.
    template <typename F, size_t RATE>
    struct SpongeMode : private::SealedSpongeMode<Absorbing<F, RATE>>,
                        private::SealedSpongeMode<Squeezing<F, RATE>> {};
    
    // The absorbing state of the `Sponge`.
    template <typename F, size_t RATE>
    struct Absorbing {
        explicit Absorbing(const SpongeRate<F, RATE>& sponge_rate) : sponge_rate(sponge_rate) {}
        explicit Absorbing(SpongeRate<F, RATE>&& sponge_rate) : sponge_rate(std::move(sponge_rate)) {}
    
        SpongeRate<F, RATE> sponge_rate;
    };
    
    // The squeezing state of the `Sponge`.
    template <typename F, size_t RATE>
    struct Squeezing {
        explicit Squeezing(const SpongeRate<F, RATE>& sponge_rate) : sponge_rate(sponge_rate) {}
        explicit Squeezing(SpongeRate<F, RATE>&& sponge_rate) : sponge_rate(std::move(sponge_rate)) {}
    
        SpongeRate<F, RATE> sponge_rate;
    };
    
    template <typename F, size_t RATE>
    Absorbing<F, RATE> init_with(const F& val) {
        return Absorbing<F, RATE>(SpongeRate<F, RATE>::from_vec(
            std::vector<std::optional<F>>(RATE, std::nullopt).tap_front(val)
        ));
    }

    
    template <typename F, template<typename, std::size_t> typename S, template<typename, std::size_t> typename M, std::size_t T, std::size_t RATE>
    class Sponge {
    public:
        using State = std::array<F, T>;
        using MdsMatrix = std::array<std::array<F, T>, T>;
        using RoundConstants = std::vector<std::array<F, T>>;
        using AbsorbingMode = M<F, RATE>;
        using SqueezingMode = S<F, RATE>;
    
    private:
        M<F, RATE> mode;
        State state;
        MdsMatrix mds_matrix;
        RoundConstants round_constants;
    
    public:
        Sponge(M<F, RATE> m, State s, MdsMatrix mds, RoundConstants rc) :
            mode(m), state(s), mds_matrix(mds), round_constants(rc) {}
    };

    template <typename F, size_t T>
    using State = std::array<F, T>;
    
    template <typename F, size_t T>
    using MdsMatrix = std::array<std::array<F, T>, T>;
    
    template <typename F, size_t RATE>
    using SpongeRate = std::array<std::unique_ptr<F>, RATE>;
    
    template <typename F, size_t RATE>
    struct Absorbing {
        SpongeRate<F, RATE> rate;
    
        Absorbing() {
            for (auto& ptr : rate) {
                ptr = std::make_unique<F>(0);
            }
        }
    
        void set(size_t i, F value) {
            rate[i] = std::make_unique<F>(value);
        }
    };
    
    template <typename F, size_t RATE>
    struct Squeezing {
        SpongeRate<F, RATE> rate;
    };
    
    template <typename F, typename S, typename M, size_t T, size_t RATE>
    class Sponge {
    private:	    
        M mode;
        State<F, T> state;
        MdsMatrix<F, T> mds_matrix;
        std::vector<std::array<F, T>> round_constants;
    
    public:
        Sponge(F initial_capacity_element) : mode(), mds_matrix(), round_constants() {
            std::tie(round_constants, mds_matrix, std::ignore) = S::constants();
            mode.rate = SpongeRate<F, RATE>();
    
            for (size_t i = 0; i < RATE; ++i) {
                mode.rate[i] = std::make_unique<F>(0);
            }
    
            state.fill(F::zero());
            state[RATE] = initial_capacity_element;
        }
    
        void absorb(F value) {
            for (auto& ptr : mode.rate) {
                if (!ptr) {
                    ptr = std::make_unique<F>(value);
                    return;
                }
            }
    
            poseidon_sponge<F, S, T, RATE>(&state, &mode.rate, &mds_matrix, &round_constants);
    
            mode = Absorbing<F, RATE>();
            mode.set(0, value);
        }
    
        Sponge<F, S, Squeezing<F, RATE>, T, RATE> finish_absorbing() {
            auto mode = poseidon_sponge<F, S, T, RATE>(&state, &this->mode.rate, &mds_matrix, &round_constants);
            return Sponge<F, S, Squeezing<F, RATE>, T, RATE>(mode, state, mds_matrix, round_constants);
        }

	F squeeze() {
            while (true) {
                for (auto& entry : mode.get().array) {
                    if (entry.has_value()) {
                        auto e = *entry;
                        entry.reset();
                        return e;
                    }
                }

                // We've already squeezed out all available elements
                mode = poseidon_sponge<F, S, T, RATE>(&state, std::nullopt, &mds_matrix, round_constants);
            }
        }
    };
    
    template <typename F, typename S, size_t T, size_t RATE>
    Sponge<F, S, Absorbing<F, RATE>, T, RATE> make_sponge(F initial_capacity_element) {
        return Sponge<F, S, Absorbing<F, RATE>, T, RATE>(initial_capacity_element);
    }

    template <typename F, size_t RATE>
    class Domain {
    public:
        // Type of the iterator that outputs padding field elements
        using Padding = std::vector<F>;
    
        // Returns the name of this domain, for debug formatting purposes
        static std::string name();
    
        // Returns the initial capacity element, encoding this domain
        static F initial_capacity_element();
    
        // Returns the padding to be appended to the input
        static Padding padding(size_t input_len);
    };


    template <typename F, size_t RATE, size_t L>
    struct ConstantLength : Domain<F, RATE> {
        using Padding = boost::iterators::take_iterator<boost::iterators::repeat_iterator<F>>;
    
        static std::string name() {
            return "ConstantLength<" + std::to_string(L) + ">";
        }
    
        static F initial_capacity_element() {
            // Capacity value is $length \cdot 2^64 + (o-1)$ where o is the output length.
            // We hard-code an output length of 1.
            return F((static_cast<uint128_t>(L) << 64));
        }
    
        static Padding padding(size_t input_len) {
            assert(input_len == L);
            // For constant-input-length hashing, we pad the input with zeroes to a multiple
            // of RATE. On its own this would not be sponge-compliant padding, but the
            // Poseidon authors encode the constant length into the capacity element, ensuring
            // that inputs of different lengths do not share the same permutation.
            size_t k = (L + RATE - 1) / RATE;
            return boost::iterators::take_iterator<boost::iterators::repeat_iterator<F>>(
                boost::iterators::repeat_iterator<F>(F::zero()), k * RATE - L
            );
        }
    };

    template <typename F>
    class FieldSpec {
    public:
        static constexpr int T = 0;
        static constexpr int RATE = 0;
        static constexpr int FULL_ROUNDS = 0;
        static constexpr int PARTIAL_ROUNDS = 0;
    };
    
    template <typename F, int T, int RATE, int FULL_ROUNDS, int PARTIAL_ROUNDS>
    class PoseidonSponge {
    public:
        PoseidonSponge() {
            // Initialize sponge
        }
    
        void absorb(const F& value) {
            // Absorb value
        }
    
        F finish_absorbing() {
            // Finish absorbing and squeeze sponge
            return F();
        }
    };
    
    template <typename F, typename S, template <typename, int> class D, int T, int RATE>
    class PoseidonHash {
    public:
        PoseidonHash() {
            // Initialize hash function
        }
    
        void init() {
            // Initialize hasher
        }
    
        F hash(const std::array<F, D<F, RATE>::L>& message) {
            for (auto value : message) {
                sponge.absorb(value);
            }
    
            auto padding = D<F, RATE>::padding(D<F, RATE>::L);
            std::for_each(padding.begin(), padding.end(), [&](const F& value) {
                sponge.absorb(value);
            });
    
            return sponge.finish_absorbing();
        }
    
    private:
        PoseidonSponge<F> sponge;
    };
    
}

#endif 