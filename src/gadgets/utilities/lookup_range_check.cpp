

template <typename F>
class RunningSum {
 public:
  explicit RunningSum(size_t W, bool strict) {
    cells_.reserve(W + 1);
    for (size_t i = 0; i <= W; ++i) {
      F val;
      if (strict && i == W) {
        val = F::zero();
      } else {
        val = F::one();
      }
      cells_.emplace_back(AssignedCell<F, F>(val));
    }
  }

  const std::vector<AssignedCell<F, F>>& operator*() const {
    return cells_;
  }

 private:
  std::vector<AssignedCell<F, F>> cells_;
};


template <typename F, typename Cell>
class RangeConstrained {
public:
    // Witnesses a subset of the bits in `value` and constrains them to be the correct
    // number of bits.
    //
    // # Panics
    //
    // Panics if `bitrange.len() >= K`.
    template <std::size_t K>
    static RangeConstrained<F, AssignedCell<F, F>> witness_short(
        const LookupRangeCheckConfig<F, K>& lookup_config,
        Layouter<F>& layouter,
        const Value<const F*>& value,
        std::size_t bitrange_start,
        std::size_t bitrange_end
    ) {
        std::size_t num_bits = bitrange_end - bitrange_start;
        assert(num_bits < K);

        // Witness the subset and constrain it to be the correct number of bits.
        auto inner = lookup_config.witness_short_check(
            layouter,
            value.map([&](const F* value){ return bitrange_subset(value, bitrange_start, bitrange_end); }),
            num_bits
        );

        return RangeConstrained<F, AssignedCell<F, F>> {
            inner,
            num_bits,
            std::default_initialization
        };
    }

private:
    RangeConstrained(
        const AssignedCell<F, F>& inner,
        std::size_t num_bits,
        std::default_initialization_t
    ) : inner(inner), num_bits(num_bits) {}

    AssignedCell<F, F> inner;
    std::size_t num_bits;
};


template <typename F, std::size_t K>
class LookupRangeCheckConfig {
public:
    LookupRangeCheckConfig(
        Selector q_lookup_,
        Selector q_running_,
        Selector q_bitshift_,
        const Column<Advice>& running_sum,
        const TableColumn& table_idx,
        std::default_initialization_t _marker
    ) : q_lookup(q_lookup_), q_running(q_running_), q_bitshift(q_bitshift_),
        running_sum(running_sum), table_idx(table_idx), _marker(_marker) {}

        meta->enable_equality(running_sum);

	// https://p.z.cash/halo2-0.1:decompose-combined-lookup
        meta->lookup([this](const auto& meta) {
            const auto q_lookup = meta.query_selector(q_lookup);
            const auto q_running = meta.query_selector(q_running);
            const auto z_cur = meta.query_advice(running_sum, Rotation::cur());

            // In the case of a running sum decomposition, we recover the word from
            // the difference of the running sums:
            //    z_i = 2^{K}⋅z_{i + 1} + a_i
            // => a_i = z_i - 2^{K}⋅z_{i + 1}
	    const auto running_sum_lookup = [this, &meta, &z_cur]() -> Expression<F> {
                const auto z_next = meta.query_advice(running_sum, Rotation::next());
                const auto running_sum_word = z_cur - (z_next * F::from(1 << K));
                return q_running.clone() * running_sum_word;
            }();

            // In the short range check, the word is directly witnessed.
            const auto short_lookup = [this, &z_cur]() -> Expression<F> {
                const auto short_word = z_cur;
                const auto q_short = Expression<F>::Constant(F::ONE) - q_running;
                return q_short * short_word;
            }();
	    // Combine the running sum and short lookups:
            return std::vector<std::pair<Expression<F>, TableColumn>>{
                { q_lookup * (running_sum_lookup + short_lookup), table_idx }
            };
        });

        // For short lookups, check that the word has been shifted by the correct number of bits.
        // https://p.z.cash/halo2-0.1:decompose-short-lookup
        meta.create_gate("Short lookup bitshift", [&](auto meta) {
            auto q_bitshift = meta.query_selector(config.q_bitshift_);
            auto word = meta.query_advice(config.running_sum_, Rotation::prev());
            auto shifted_word = meta.query_advice(config.running_sum_, Rotation::cur());
            auto inv_two_pow_s = meta.query_advice(config.running_sum_, Rotation::next());

            auto two_pow_k = F::from(1 << K);

            // shifted_word = word * 2^{K-s}
            //              = word * 2^K * inv_two_pow_s
            return Constraints(q_bitshift, (word * two_pow_k * inv_two_pow_s - shifted_word));
        });

        return config;
    }

    void load(Layouter<F>& layouter) {
        layouter.assign_table("table_idx", [&](auto& table) {
            for (std::size_t index = 0; index < (1 << K); ++index) {
                table.assign_cell(
                    this->table_idx_,
                    index,
                    [&]() { return Value(F::from(index)); }
                );
            }
            return Ok();
        });
    }

    /// Range check on an existing cell that is copied into this helper.
    ///
`    /// Returns an error if `element` is not in a column that was passed to
    /// `ConstraintSystem::enable_equality` during circuit configuration.
    RunningSum<F> copy_check(Layouter<F>& layouter, AssignedCell<F, F> element, std::size_t num_words, bool strict) {
        return element.copy_check(layouter, num_words, strict);
    }

    Result<RunningSum<F>, Error> copy_check(
        Layouter<F>& layouter,
        const AssignedCell<F>& element,
        size_t num_words,
        bool strict
    ) const {
        return layouter.assign_region(
            [&] { return std::to_string(num_words) + " words range check"; },
            [&](Region<F>& region) -> Result<RunningSum<F>, Error> {
                // Copy `element` and initialize running sum `z_0 = element` to decompose it.
                AssignedCell<F> z_0 = element.template copy_advice<0>(
                    [&] { return "z_0"; },
                    region,
                    this->running_sum
                );
                return this->range_check(region, z_0, num_words, strict);
            }
        );
    }

    Result<RunningSum<F>, Error> witness_check(
        Layouter<F>& layouter,
        const F& value,
        size_t num_words,
        bool strict
    ) {
        return layouter.assign_region(
            "Witness element",
            [&](Region<F> &region) -> Result<RunningSum<F>, Error> {
                auto z_0 = region.assign_advice(
                    "Witness element", self->running_sum, 0, [&]() { return value; }
                );
                return self->range_check(region, z_0, num_words, strict);
            }
        );
    }

    RunningSum<F> range_check(
        std::unique_ptr<bellman::Worker> worker,
        const LinearCombination<F>& element,
        std::size_t num_words,
        bool strict
    ) {
        // `num_words` must fit into a single field element.
        assert(num_words * K <= F::CAPACITY);

        std::size_t num_bits = num_words * K;

        // Chunk the first num_bits bits into K-bit words.
        std::vector<std::vector<F>> words;
        {
            // Take first num_bits bits of `element`.
            std::vector<bool> bits;
            element.evaluate(&worker, &mut |var, coeff| {
                if bits.len() < num_bits {
                    let var_bits = var.into_bits_le(worker);
                    bits.extend(var_bits);
                }
            });

            std::vector<F> word;
            for (std::size_t i = 0; i < bits.size(); i += K) {
                std::array<bool, K> chunk;
                std::copy(bits.begin() + i, bits.begin() + i + K, chunk.begin());
                word.push_back(F::from_le_bytes(&chunk));
            }
            words = std::vector(word.begin(), word.end());
        }

        std::vector<LinearCombination<F>> zs;
        LinearCombination<F> z = element.clone();
        F inv_two_pow_k = F::from(1u64 << K).invert().unwrap();
        for (std::size_t idx = 0; idx < words.size(); ++idx) {
            // Enable q_lookup on this row
            this->q_lookup.enable(&worker, idx)?;

            // Enable q_running on this row
            this->q_running.enable(&worker, idx)?;

            // z_next = (z_cur - m_cur) / 2^K
            z = (z - words[idx]) * inv_two_pow_k;
            LinearCombination<F> z_next = this->running_sum.advice(idx + 1, || z)?;
            zs.push_back(z_next.clone());
        }

        if (strict) {
            // Constrain the final `z` to be zero.
            worker->constrain(zs.back().cell() - F::ZERO);
        }

        return RunningSum<F>(std::move(zs));
    }
    
    template <typename F, std::size_t K>
    void copy_short_check(
        Layouter<F>& layouter,
        const AssignedCell<F>& element,
        std::size_t num_bits
    ) const {
        assert(num_bits < K && "num_bits must be less than K");

        layouter.assign_region(
            [&]() -> std::string {
                return "Range check " + std::to_string(num_bits) + " bits";
            },
            [&](Region<F> &region) -> void {
                AssignedCell<F> copy_element = element.template copy_advice<0>(
                    region,
                    this->running_sum
                );

                this->short_range_check(region, copy_element, num_bits);
            }
        );
    }


    AssignedCell<F, F> witness_short_check(Layouter<F>& layouter, Value<F> element, std::size_t num_bits) {
        assert(num_bits <= K);

        return layouter.assign_region(
            [num_bits, &element]() -> std::string {
                return "Range check " + std::to_string(num_bits) + " bits";
            },
            [&](Region<F>& region) -> AssignedCell<F, F> {
                AssignedCell<F, F> element_cell = region.assign_advice(
                    [&]() -> std::string {
                        return "Witness element";
                    },
                    this->running_sum,
                    0,
                    [&]() -> Value<F> {
                        return element;
                    });

                this->short_range_check(&region, element_cell.clone(), num_bits);

                return element_cell;
            });
    }

    // Constrain `x` to be a NUM_BITS word.
    //
    // `element` must have been assigned to `self.running_sum` at offset 0.
    void short_range_check(
        zk::snark::r1cs_variable_assignment<F>& assignment,
        const F& element,
        std::size_t num_bits
    ) {
        // Enable lookup for `element`, to constrain it to 10 bits.
        q_lookup.enable(assignment, 0);

        // Enable lookup for shifted element, to constrain it to 10 bits.
        q_lookup.enable(assignment, 1);

        // Check element has been shifted by the correct number of bits.
        q_bitshift.enable(assignment, 1);

        // Assign shifted `element * 2^{K - num_bits}`
        auto shifted = element * F(1 << (K - num_bits));

        assignment.emplace(this->running_sum, 1, shifted);

        // Assign 2^{-num_bits} from a fixed column.
        auto inv_two_pow_s = F(1 << num_bits).inversed();

        assignment.emplace_constant(this->running_sum, 2, inv_two_pow_s);
    }    


private:
    Selector q_lookup;
    Selector q_running;
    Selector q_bitshift;
    Column<Advice> running_sum;
    TableColumn table_idx;
    std::default_initialization_t _marker;

};



