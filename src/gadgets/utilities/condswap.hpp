#ifndef SG_HALO2_GADGETS_UTILITIES_CONDSWAP_HPP
#define SG_HALO2_GADGETS_UTILITIES_CONDSWAP_HPP

#include <halo2_proofs/circuit/chip.hpp>
#include <halo2_proofs/circuit/column.hpp>
#include <halo2_proofs/circuit/selector.hpp>

using namespace halo2_proofs;
using namespace halo2_proofs::circuit;

namespace halo2::gadgets::utilities {

    template <typename F>
    class CondSwapInstructions : public UtilitiesInstructions<F> {	
    public:
        using Var = typename UtilitiesInstructions<F>::Var;	    

	std::pair<Var, Var> swap(Layouter<F>& layouter, std::pair<Var, Value<F>> pair, Value<bool> swap);
    };	    
    
    template <typename F>
    class CondSwapConfig {
    public:
        Selector q_swap;
        Column<Advice> a;
        Column<Advice> b;
        Column<Advice> a_swapped;
        Column<Advice> b_swapped;
        Column<Advice> swap;
    };

    template <typename F>
    class CondSwapChip : public Chip<F>, public UtilitiesInstructions<F>, public CondSwapInstructions<F> { 	
    public:
	using Config = CondSwapConfig<F>;
	using Loaded = std::nullptr_t;

	CondSwapChip(const CondSwapConfig<F>& config);
	const Config& config() const override;
	const Loaded& loaded() const override;

        static CondSwapConfig<F> configure(ConstraintSystem<F>& meta,
                                      std::array<Column<Advice>, 5> advices);
	std::pair<Var, Var> swap(Layouter<F>& layouter, std::pair<Var, F>& pair, bool swap) override;
        

    private:
	Config config;
        CondSwapConfig<F> config_;
        std::tuple<PrimeField> _marker;
    };

} // namespace halo2::gadgets::utilities

#endif  // SG_HALO2_GADGETS_UTILITIES_CONDSWAP_HPP
