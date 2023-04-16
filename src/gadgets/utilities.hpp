#ifndef SG_HALO2_UTILITIES_HPP
#define SG_HALO2_UTILITIES_HPP

namespace halo2::gadgets {

    template <typename F>
    class FieldValue {
    public:
        virtual Value<const F>* value() const = 0;
        virtual ~FieldValue() {}
    };

    template <typename F>
    class Value : public FieldValue<F> {
    public:
        Value(const F& f) : field_(f) {}
        Value(const Value<F>& v) : field_(v.field_) {}
        Value(const AssignedCell<F, F>& ac) : field_(ac.value()) {}
        virtual Value<const F>* value() const override { return new Value<const F>(*this); }
        const F& as_ref() const { return field_; }
    
    private:
        F field_;
    };

    template <typename F>
    class AssignedCell : public FieldValue<F> {
    public:
        AssignedCell(const F& f) : field_(f) {}
        virtual Value<const F>* value() const override { return new Value<const F>(field_); }
        const F& value() const { return field_; }
    private:
        F field_;
    };
    

    // Utilities Instructions
    template <typename F>
    class UtilitiesInstructions {
    public:
        // Variable in the circuit.
        using Var = ::Var<F>;
    
        // Load a variable.
        Var load_private(Layouter<F>& layouter, const Column<ColumnType::Advice>& column, const Value<F>& value);
    };

    // A type representing a range-constrained field element.
    template <typename F, typename T>
    class RangeConstrained {
    public:
        RangeConstrained(const T& inner, const std::size_t num_bits);
    
        const T& inner();
        std::size_t num_bits() const; 
    
    private:
        T inner_;
        std::size_t num_bits_;
        PhantomData<F> _phantom;
    };

    // Constructs a `RangeConstrained<Value<F>>` as a bitrange of the given value.
    template <typename F>
    RangeConstrained<F, Value<F>> bitrange_of(const Value<const PrimeFieldBits*>& value, const std::size_t bitrange_start, const std::size_t bitrange_end) {
        const std::size_t num_bits = bitrange_end - bitrange_start;
        assert(num_bits <= sizeof(F) * 8);
        assert(bitrange_end <= sizeof(*value) * 8);
        auto bitrange_subset = ranges::subrange(bitrange_start, bitrange_end);
        auto inner = value | ranges::views::all | ranges::views::transform([&](const PrimeFieldBits& field) {
            F result = 0;
            ranges::copy(field.get_bits(bitrange_subset), result.get_bits(bitrange_subset).begin());
            return Value<F>(std::make_unique<F>(std::move(result)));
        });
        return RangeConstrained<F, decltype(inner)>(std::move(inner), num_bits);
    }

    template<typename F>
    RangeConstrained<F, AssignedCell<F, F>> unsound_unchecked(AssignedCell<F, F> cell, size_t num_bits) {
        return RangeConstrained<F, AssignedCell<F, F>>(cell, num_bits);
    }

    template<typename F>
    Expression<F> bool_check(Expression<F> value) {
        return range_check(value, 2);
    }

    template<typename F>
    Expression<F> ternary(Expression<F> a, Expression<F> b, Expression<F> c) {
        auto one_minus_a = Expression<F>::Constant(F::ONE) - a;
        return a * b + one_minus_a * c;
    }

    template <typename F>
    F bitrange_subset(const F& field_elem, const std::pair<size_t, size_t>& bitrange) {
        assert(bitrange.second <= F::NUM_BITS);
    
        auto le_bits = field_elem.to_le_bits();
        F acc = F::ZERO;
    
        for (size_t i = bitrange.first; i < bitrange.second; i++) {
            bool bit = le_bits[i];
            acc = acc.double();
    
            if (bit) {
                acc += F::ONE;
            }
        }
    
        return acc;
    }
    // Note that C++ doesn't have the clone() method like Rust, so I assumed that clone() is a member 
    // function of the Expression<F> class. If that's not the case, you'll need to replace clone() 
    // with the appropriate member function or copy constructor.
    template <typename F>
    Expression<F> range_check(Expression<F> word, size_t range) {
        Expression<F> acc = word.clone();
        for (size_t i = 1; i < range; ++i) {
            auto val = F::from(i);
            acc = acc * (Expression::Constant(val) - word.clone());
        }
        return acc;
    }

    // Note that in C++, we need to use std::vector to represent dynamic arrays. Also, in C++, we need to 
    // use std::copy and std::back_inserter to append elements to a vector. Finally, I used std::advance 
    // to move the iterator forward, and std::for_each to iterate over the bits in each chunk.
    template <typename F>
    std::vector<uint8_t> decompose_word(const F& word, size_t word_num_bits, size_t window_num_bits) {
        assert(window_num_bits <= 8);
    
        // Pad bits to multiple of window_num_bits
        size_t padding = (window_num_bits - (word_num_bits % window_num_bits)) % window_num_bits;
        std::vector<bool> bits;
        auto word_bits = word.to_le_bits();
        bits.reserve(word_num_bits + padding);
        std::copy(word_bits.begin(), word_bits.begin() + word_num_bits, std::back_inserter(bits));
        std::fill_n(std::back_inserter(bits), padding, false);
        assert(bits.size() == word_num_bits + padding);
    
        std::vector<uint8_t> result;
        result.reserve(bits.size() / window_num_bits);
        for (auto chunk = bits.begin(); chunk != bits.end(); std::advance(chunk, window_num_bits)) {
            uint8_t value = 0;
            auto end = std::next(chunk, window_num_bits);
            std::for_each(chunk, end, [&](const bool& b) {
                value = (value << 1) + b;
            });
            result.push_back(value);
        }
        return result;
    }

    template <size_t L>
    uint64_t lebs2ip(const std::array<bool, L>& bits) {
        static_assert(L <= 64, "L must be <= 64");
    
        uint64_t result = 0;
        for (size_t i = 0; i < L; ++i) {
            if (bits[i]) {
                result += 1ull << i;
            }
        }
        return result;
    }

    template <size_t NUM_BITS>
    std::array<bool, NUM_BITS> i2lebsp(uint64_t value) {
        static_assert(NUM_BITS <= 64, "NUM_BITS must be <= 64");
    
        auto gen_const_array = [](auto closure) {
            std::array<std::decay_t<decltype(closure(0))>, NUM_BITS> ret{};
            for (size_t bit = 0; bit < NUM_BITS; ++bit) {
                ret[bit] = closure(bit);
            }
            return ret;
        };
    
        return gen_const_array([&](size_t mask) { return (value & (1ull << mask)) != 0; });
    }
    


} // halo2::gadgets

#endif  //SG_HALO2_UTILITIES_HPP
