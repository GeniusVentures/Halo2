#include <halo2_proofs/circuit/layouter.hpp>
#include <halo2_proofs/plonk/error.hpp>
#include <halo2_proofs/circuit/chip.hpp>
#include <halo2_proofs/circuit/column.hpp>
#include <halo2_proofs/circuit/selector.hpp>
#include <group/ff.hpp>
#include <memory>

using namespace halo2_proofs;
using namespace halo2_proofs::circuit;
using namespace halo2_proofs::plonk;
using namespace group::ff;

template <typename F>
class CondSwapInstructions : public UtilitiesInstructions<F> {
public:
    using Var = typename UtilitiesInstructions<F>::Var;

    std::pair<Var, Var> swap(Layouter<F>& layouter, std::pair<Var, Value<F>> pair, Value<bool> swap) {
        Var a = pair.first;
        Var b = layouter.assign(Some(pair.second))?;

        Var tmp = layouter.assign(None)?;
        layouter.assign_conditional_region(|| swap.get() == Some(true), |region| {
            region.assign(tmp, a)?;
            region.assign(a, b)?;
            region.assign(b, tmp)?;
            Ok(())
        })?;

        return {a, b};
    }
};


template <typename F>
struct CondSwapConfig {
    Selector q_swap;
    Column<Advice> a;
    Column<Advice> b;
    Column<Advice> a_swapped;
    Column<Advice> b_swapped;
    Column<Advice> swap;
};

// Helper function for creating a CondSwapConfig instance with default column names.
template <typename F>
CondSwapConfig<F> make_default_cond_swap_config() {
    return {
        Selector::new("q_swap"),
        Column<Advice>::new("a"),
        Column<Advice>::new("b"),
        Column<Advice>::new("a_swapped"),
        Column<Advice>::new("b_swapped"),
        Column<Advice>::new("swap"),
    };
}

template <typename F>
class CondSwapChip {
 public:
  using PrimeField = F;

  explicit CondSwapChip(const CondSwapConfig<F>& config)
      : config_(config), _marker() {}

  static CondSwapConfig<F> configure(ConstraintSystem<F>& meta,
                                      std::array<Column<Advice>, 5> advices) {
    auto a = advices[0];
    meta.enable_equality(a);

    auto q_swap = meta.selector();

    auto config = CondSwapConfig<F>{
        q_swap,
        a,
        advices[1],
        advices[2],
        advices[3],
        advices[4],
    };

    meta.create_gate("a' = b * swap + a * (1 - swap)", [&](auto& meta) {
      auto q_swap = meta.query_selector(config.q_swap);

      auto a = meta.query_advice(config.a, Rotation::cur());
      auto b = meta.query_advice(config.b, Rotation::cur());
      auto a_swapped = meta.query_advice(config.a_swapped, Rotation::cur());
      auto b_swapped = meta.query_advice(config.b_swapped, Rotation::cur());
      auto swap = meta.query_advice(config.swap, Rotation::cur());

      auto a_check = a_swapped - ternary(swap.clone(), b.clone(), a.clone());

      auto b_check = b_swapped - ternary(swap.clone(), a, b);

      auto bool_check = bool_check(swap);

      return Constraints<>(q_swap, {
        {"a check", a_check},
        {"b check", b_check},
        {"swap is bool", bool_check},
      });
    });

    return config;
  }

 private:
};


template <typename F>
class CondSwapChip : public Chip<F>, public UtilitiesInstructions<F>, public CondSwapInstructions<F> {
public:
    using PrimeField = F;
    using Var = AssignedCell<F, F>;
    using Config = CondSwapConfig<F>;
    using Loaded = std::nullptr_t;
    using PrimeField = F;

    explicit CondSwapChip(const CondSwapConfig<F>& config)
      : config_(config), _marker() {}


    const Config& config() const override {
        return config_;
    }

    const Loaded& loaded() const override {
        static std::nullptr_t dummy;
        return dummy;
    }

    static CondSwapConfig<F> configure(ConstraintSystem<F>& meta,
                                      std::array<Column<Advice>, 5> advices) {
    auto a = advices[0];
    meta.enable_equality(a);

    auto q_swap = meta.selector();

    auto config = CondSwapConfig<F>{
        q_swap,
        a,
        advices[1],
        advices[2],
        advices[3],
        advices[4],
    };

    meta.create_gate("a' = b * swap + a * (1 - swap)", [&](auto& meta) {
      auto q_swap = meta.query_selector(config.q_swap);

      auto a = meta.query_advice(config.a, Rotation::cur());
      auto b = meta.query_advice(config.b, Rotation::cur());
      auto a_swapped = meta.query_advice(config.a_swapped, Rotation::cur());
      auto b_swapped = meta.query_advice(config.b_swapped, Rotation::cur());
      auto swap = meta.query_advice(config.swap, Rotation::cur());

      auto a_check = a_swapped - ternary(swap.clone(), b.clone(), a.clone());

      auto b_check = b_swapped - ternary(swap.clone(), a, b);

      auto bool_check = bool_check(swap);

      return Constraints<>(q_swap, {
        {"a check", a_check},
        {"b check", b_check},
        {"swap is bool", bool_check},
      });
    });

    return config;
  }

    // Implementation of the swap function for conditional swap
    std::pair<Var, Var> swap(Layouter<F>& layouter, std::pair<Var, F>& pair, bool swap) override {
        const auto config = this->config();

        layouter.assign_region(
            "swap",
            [&](auto& region) -> Result<std::pair<Var, Var>, Error> {
                // Enable `q_swap` selector
                config.q_swap.enable(region, 0);

                // Copy in `a` value
                auto a = pair.first.copy_advice(
                    "copy a",
                    region,
                    config.a,
                    0
                );

                // Witness `b` value
                auto b = region.assign_advice(
                    "witness b",
                    config.b,
                    0,
                    [&pair]() { return pair.second; }
                );

                // Witness `swap` value
                auto swap_val = swap ? F::one() : F::zero();
                region.assign_advice("swap", config.swap, 0, [&swap_val]() { return swap_val; });

                // Conditionally swap a
                auto a_swapped = region.assign_advice(
                    "a_swapped",
                    config.a_swapped,
                    0,
                    [&a, &b, &swap]() {
                        return (swap) ? b.value() : a.value();
                    }
                );

                // Conditionally swap b
                auto b_swapped = region.assign_advice(
                    "b_swapped",
                    config.b_swapped,
                    0,
                    [&a, &b, &swap]() {
                        return (swap) ? a.value() : b.value();
                    }
                );

                // Return swapped pair
                return std::make_pair(a_swapped, b_swapped);
            }
        );
    }
private:
    Config config;
    CondSwapConfig<F> config_;
    std::tuple<PrimeField> _marker;
};

