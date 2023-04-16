

namespace halo2::gadgets {
    
   // UtilitiesInstructions
    Var UtilitiesInstructions::load_private(Layouter<F>& layouter, const Column<ColumnType::Advice>& column, const Value<F>& value) {
        std::unique_ptr<std::function<Var(std::size_t)>> callback(
            new std::function<Var(std::size_t)>([column, value](std::size_t region_index) -> Var {
                Region<F> region(region_index);
                return Var::from(region.assign_advice(column, 0, value.value()));
            })
        );
        layouter.assign_region("load private", std::move(callback));
    }

    // RangeConstrained
    RangeConstrained::RangeConstrained(const T& inner, const std::size_t num_bits) : inner_(inner), num_bits_(num_bits) {}
    const T& RangeConstrained::inner() const { return inner_; }
    std::size_t RangeConstrained::num_bits() const { return num_bits_; }
}
