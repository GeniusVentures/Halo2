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


namespace halo2::gadgets::utilities {

    std::pair<Var, Var> CondSwapInstructions::swap(Layouter<F>& layouter, std::pair<Var, Value<F>> pair, Value<bool> swap) {
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

    template <typename F>
    CondSwapConfig<F>::CondSwapConfig() {
	q_swap = new Selector("q_swap");
	a = new Column<Advice>("a");
	b = new Column<Advice>("b");
	a_swapped = new Column<Advice>("a_swapped");
	b_swapped = new Column<Advice>("b_swapped");
	swap = new Column<Advice>("swap")
    }

    CondSwapChip::CondSwapChip(const CondSwapConfig<F>& config)
      : config_(config), _marker() {}


    Config& CondSwapChip::config() const override {
        return config_;
    }

    Loaded& CondSwapChip::loaded() const override {
        static std::nullptr_t dummy;
        return dummy;
    }

    CondSwapConfig<F> CondSwapChip::configure(ConstraintSystem<F>& meta,
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
    std::pair<Var, Var> CondSwapChip::swap(Layouter<F>& layouter, std::pair<Var, F>& pair, bool swap) {
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
}
