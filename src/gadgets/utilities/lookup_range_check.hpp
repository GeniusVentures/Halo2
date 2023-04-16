#ifndef SG_HALO2_GADGETS_UTILITIES_LOOKUP_RANGE_CHECK_HPP
#define SG_HALO2_GADGETS_UTILITIES_LOOKUP_RANGE_CHECK_HPP

#include <cstddef> // for std::size_t
#include <memory> // for std::unique_ptr
#include <cassert> // for assert
#include <range/v3/view/subrange.hpp> // for ranges::subrange
#include <range/v3/algorithm/copy.hpp> // for ranges::copy
#include <range/v3/utility/static_const.hpp> // for ranges::views::all

namespace halo2::gadgets::utilities {

    template <typename F, typename Cell>
    class RangeConstrained {
    public:
	RangeConstrained(
            const AssignedCell<F, F>& inner,
            std::size_t num_bits,
            std::default_initialization_t
        );

        static RangeConstrained<F, AssignedCell<F, F>> witness_short(
            const LookupRangeCheckConfig<F, K>& lookup_config,
            Layouter<F>& layouter,
            const Value<const F*>& value,
            std::size_t bitrange_start,
            std::size_t bitrange_end
        );

    private:
	AssignedCell<F, F> inner;
        std::size_t num_bits;
    
    };

    template <typename F, std::size_t K>
    class LookupRangeCheckConfig {
	LookupRangeCheckConfig(
            Selector q_lookup_,
            Selector q_running_,
            Selector q_bitshift_,
            const Column<Advice>& running_sum,
            const TableColumn& table_idx,
            std::default_initialization_t _marker
        );

	void load(Layouter<F>& layouter);
	RunningSum<F> copy_check(Layouter<F>& layouter, AssignedCell<F, F> element, std::size_t num_words, bool strict);
	Result<RunningSum<F>, Error> copy_check(
            Layouter<F>& layouter,
            const AssignedCell<F>& element,
            size_t num_words,
            bool strict
        ) const;

	Result<RunningSum<F>, Error> witness_check(
            Layouter<F>& layouter,
            const F& value,
            size_t num_words,
            bool strict
        );

	RunningSum<F> range_check(
            std::unique_ptr<bellman::Worker> worker,
            const LinearCombination<F>& element,
            std::size_t num_words,
            bool strict
        );

	template <typename F, std::size_t K>
        void copy_short_check(
            Layouter<F>& layouter,
            const AssignedCell<F>& element,
            std::size_t num_bits
        );

	AssignedCell<F, F> witness_short_check(Layouter<F>& layouter, Value<F> element, std::size_t num_bits);
	    void short_range_check(
            zk::snark::r1cs_variable_assignment<F>& assignment,
            const F& element,
            std::size_t num_bits
        );
	
	void short_range_check(
            zk::snark::r1cs_variable_assignment<F>& assignment,
            const F& element,
            std::size_t num_bits
        )

    private:
	Selector q_lookup;
        Selector q_running;
        Selector q_bitshift;
        Column<Advice> running_sum;
        TableColumn table_idx;
        std::default_initialization_t _marker;

    };	    

}

#endif // SG_HALO2_GADGETS_UTILITIES_LOOKUP_RANGE_CHECK_HPP
