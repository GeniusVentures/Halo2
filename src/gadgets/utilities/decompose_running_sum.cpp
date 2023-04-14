#include <vector>
#include <cstddef>
#include <memory>
#include <cstdint>
#include <cassert>
#include <algorithm>


namespace halo2::gadgets::utilities {

    RunningSum::RunningSum(std::size_t size) : data_(size) {}
    std::vector<AssignedCell<F, F>>& RunningSum::operator*() const {
        return data_;
    }

    RunningSumConfig::RunningSumConfig() {}
    RunningSumConfig::RunningSumConfig(Column<Advice> z, Selector q_range_check) :
        q_range_check_(std::move(q_range_check)), z_(std::move(z)) {}

    // Constructor
    RunningSumConfig::RunningSumConfig(ColumnType z_column, libsnark::pb_variable<F> q_range_check_var)
        : z(std::move(z_column)), q_range_check(q_range_check_var) {}

    // Returns the q_range_check selector of this RunningSumConfig.
    Selector RunningSumConfig::q_range_check() const {
        return q_range_check_;
    }

    // Configures the RunningSumConfig instance
    RunningSumConfig<F, WINDOW_NUM_BITS> RunningSumConfig::configure(
        libsnark::protoboard<F>& pb,
        libsnark::pb_variable<F> q_range_check,
        const libsnark::pb_variable_array<F>& z
    ) {
        assert(WINDOW_NUM_BITS <= 3);

        // Enable equality for column `z`
        pb.set_input_sizes(1);
        pb.set_input_sizes(0, 1);
        pb.enable_equality(z[0]);

        // Create a RunningSumConfig instance
        RunningSumConfig<F, WINDOW_NUM_BITS> config(
            z[0],
            q_range_check
        );

        // Create a range check gate for the RunningSumConfig instance
        pb.add_r1cs_constraint(
            libsnark::r1cs_constraint<F>(libsnark::linear_combination<F>(q_range_check), 1,
                                         libsnark::linear_combination<F>(config.z) - libsnark::linear_combination<F>(config.z) * F(1ull << WINDOW_NUM_BITS)),
            "range check"
        );

        return config;
    }
    // Returns the `z` column of this RunningSumConfig instance
    ColumnType RunningSumConfig::get_z() const {
        return z;
    }

    // Returns the `q_range_check` selector of this RunningSumConfig instance
    libsnark::pb_variable<F> RunningSumConfig::get_q_range_check() const {
        return q_range_check_;
    }

    // Decomposes a field element `alpha` that is witnessed in this helper
    RunningSum<F> RunningSumConfig::witness_decompose(
        libsnark::pb_variable_array<F>& input,
        const size_t offset,
        const F alpha,
        const bool strict,
        const size_t word_num_bits,
        const size_t num_windows
    ) const {
        // Assign `alpha` to `z[0]` in the input
        input[offset] = alpha;

        // Create a new RunningSum instance using `decompose` function
        return this->decompose(input, offset, strict, word_num_bits, num_windows);
    }

    RunningSum<F> RunningSumConfig::copy_decompose(
        Region<F>& region,
        size_t offset,
        const AssignedCell<F>& alpha,
        bool strict,
        size_t word_num_bits,
        size_t num_windows
    ) {
        auto z_0 = alpha.copy_advice(region, this->ca, offset, "copy z_0 = alpha");
        return this->decompose(region, offset, z_0, strict, word_num_bits, num_windows);
    }

    /**
     * `z_0` must be the cell at `(self.z, offset)` in `region`.
     *
     * # Panics
     *
     * Panics if there are too many windows for the given word size.
     */
    Result<RunningSum<F>, Error> RunningSumConfig::decompose(
        Region<F> *region,
        size_t offset,
        const AssignedCell<F> &z_0,
        bool strict,
        size_t word_num_bits,
        size_t num_windows
    ) {
        // Make sure that we do not have more windows than required for the number
        // of bits in the word. In other words, every window must contain at least
        // one bit of the word (no empty windows).
        //
        // For example, let:
        //      - word_num_bits = 64
        //      - WINDOW_NUM_BITS = 3
        // In this case, the maximum allowed num_windows is 22:
        //                    3 * 22 < 64 + 3
        //
        assert(WINDOW_NUM_BITS * num_windows < word_num_bits + WINDOW_NUM_BITS);

        // Enable selectors
        for (size_t idx = 0; idx < num_windows; ++idx) {
            this->q_range_check.enable(region, offset + idx).expect("Failed to enable selectors");
        }

        // Decompose base field element into K-bit words.
        auto words = z_0.value().map([&word_num_bits, &num_windows](const auto &word) {
            return super::decompose_word<F>(word, word_num_bits, WINDOW_NUM_BITS, num_windows);
        });

        // Initialize empty vector to store running sum values [z_0, ..., z_W].
        std::vector<AssignedCell<F>> zs{z_0.clone()};
        AssignedCell<F> z = z_0;

        // Assign running sum `z_{i+1}` = (z_i - k_i) / (2^K) for i = 0..=n-1.
        // Outside of this helper, z_0 = alpha must have already been loaded into the
        // `z` column at `offset`.
        const auto two_pow_k_inv = Value::known(F::from(1ULL << WINDOW_NUM_BITS).invert().expect("Failed to compute 2^K inverse"));
        for (size_t i = 0; i < num_windows; ++i) {
            // z_next = (z_cur - word) / (2^K)
            const auto &word = words[i];
            auto z_next_val = (z.value().get() - word.get()) * two_pow_k_inv.get();
            auto z_next = region->assign_advice(
                [&i]() { return "z_" + std::to_string(i + 1); },
                this->z,
                offset + i + 1,
                [&z_next_val]() { return z_next_val; }
            ).expect("Failed to assign advice cell for z");

            // Update `z`.
            z = z_next;
            zs.push_back(z.clone());
        }
        assert(zs.size() == num_windows + 1);

        if (strict) {
            // Constrain the final running sum output to be zero.
            region->constrain_constant(zs.back().cell(), F::ZERO).expect("Failed to constrain the final running sum output to be zero");
        }

        return Ok(RunningSum<F>(std::move(zs)));
    }

    /**
     * `z_0` must be the cell at `(self.z, offset)` in `region`.
     *
     * # Panics
     *
     * Panics if there are too many windows for the given word size.
     */
    Result<RunningSum<F>, Error> RunningSumConfig::decompose(
        Region<F> *region,
        size_t offset,
        const AssignedCell<F> &z_0,
        bool strict,
        size_t word_num_bits,
        size_t num_windows
    ) {
        // Make sure that we do not have more windows than required for the number
        // of bits in the word. In other words, every window must contain at least
        // one bit of the word (no empty windows).
        //
        // For example, let:
        //      - word_num_bits = 64
        //      - WINDOW_NUM_BITS = 3
        // In this case, the maximum allowed num_windows is 22:
        //                    3 * 22 < 64 + 3
        //
        assert(WINDOW_NUM_BITS * num_windows < word_num_bits + WINDOW_NUM_BITS);

        // Enable selectors
        for (size_t idx = 0; idx < num_windows; ++idx) {
            this->q_range_check.enable(region, offset + idx).expect("Failed to enable selectors");
        }

        // Decompose base field element into K-bit words.
        auto words = z_0.value().map([&word_num_bits, &num_windows](const auto &word) {
            return super::decompose_word<F>(word, word_num_bits, WINDOW_NUM_BITS, num_windows);
        });

        // Initialize empty vector to store running sum values [z_0, ..., z_W].
        std::vector<AssignedCell<F>> zs{z_0.clone()};
        AssignedCell<F> z = z_0;

        // Assign running sum `z_{i+1}` = (z_i - k_i) / (2^K) for i = 0..=n-1.
        // Outside of this helper, z_0 = alpha must have already been loaded into the
        // `z` column at `offset`.
        const auto two_pow_k_inv = Value::known(F::from(1ULL << WINDOW_NUM_BITS).invert().expect("Failed to compute 2^K inverse"));
        for (size_t i = 0; i < num_windows; ++i) {
            // z_next = (z_cur - word) / (2^K)
            const auto &word = words[i];
            auto z_next_val = (z.value().get() - word.get()) * two_pow_k_inv.get();
            auto z_next = region->assign_advice(
                [&i]() { return "z_" + std::to_string(i + 1); },
                this->z,
                offset + i + 1,
                [&z_next_val]() { return z_next_val; }
            ).expect("Failed to assign advice cell for z");

            // Update `z`.
            z = z_next;
            zs.push_back(z.clone());
        }
        assert(zs.size() == num_windows + 1);

        if (strict) {
            // Constrain the final running sum output to be zero.
            region->constrain_constant(zs.back().cell(), F::ZERO).expect("Failed to constrain the final running sum output to be zero");
        }

        return Ok(RunningSum<F>(std::move(zs)));
    }
}
