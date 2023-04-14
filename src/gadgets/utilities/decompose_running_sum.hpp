#ifndef SG_HALO2_GADGETS_UTILITIES_DECOMPOSE_RUNNING_SUM_HPP
#define SG_HALO2_GADGETS_UTILITIES_DECOMPOSE_RUNNING_SUM_HPP

namespace halo2::gadgets::utilities {

    template <typename F>
    class RunningSum {	
    public:	    
        RunningSum(std::size_t size);    
	const std::vector<AssignedCell<F, F>>& operator*() const;
    private:
        std::vector<AssignedCell<F, F>> data_;	
    };	

    template <typename F, std::size_t WINDOW_NUM_BITS>
    class RunningSumConfig {
    public:
	RunningSumConfig();
	RunningSumConfig(Column<Advice> z, Selector q_range_check);
	RunningSumConfig(ColumnType z_column, libsnark::pb_variable<F> q_range_check_var);
        
        Selector q_range_check() const;	
	static RunningSumConfig<F, WINDOW_NUM_BITS> configure(
            libsnark::protoboard<F>& pb,
            libsnark::pb_variable<F> q_range_check,
            const libsnark::pb_variable_array<F>& z
        );
	ColumnType get_z() const;
	libsnark::pb_variable<F> get_q_range_check() const;
	RunningSum<F> witness_decompose(
            libsnark::pb_variable_array<F>& input,
            const size_t offset,
            const F alpha,
            const bool strict,
            const size_t word_num_bits,
            const size_t num_windows
        ) const;
	RunningSum<F> copy_decompose(
            Region<F>& region,
            size_t offset,
            const AssignedCell<F>& alpha,
            bool strict,
            size_t word_num_bits,
            size_t num_windows
        );
	Result<RunningSum<F>, Error> decompose(
            Region<F> *region,
            size_t offset,
            const AssignedCell<F> &z_0,
            bool strict,
            size_t word_num_bits,
            size_t num_windows
        );

    private:	    
	Selector q_range_check_;
        Column<Advice> z_;
        std::unique_ptr<std::remove_const_t<F>> _marker_;
    };	    
}

#endif // SG_HALO2_GADGETS_UTILITIES_DECOMPOSE_RUNNING_SUM_HPP 
