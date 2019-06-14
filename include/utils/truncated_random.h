//
// Created by Amir Masoud Abdol on 2019-06-14.
//

#ifndef SAMPP_TRUNCATED_RANDOM_H
#define SAMPP_TRUNCATED_RANDOM_H

#include <random>
#include <iostream>

template<class _RealType = double>
class truncated_normal_distribution {
public:
    // types
    typedef _RealType result_type;

    class param_type
    {
        result_type mean_;
        result_type stddev_;
        result_type min_;
        result_type max_;
    public:
        typedef truncated_normal_distribution distribution_type;

        explicit param_type(result_type mean = 0, result_type stddev = 1,
                result_type min = -std::numeric_limits<_RealType>::infinity(),
                result_type max = std::numeric_limits<_RealType>::infinity())
                : mean_(mean), stddev_(stddev), min_(min), max_(max) {}

        result_type mean() const {return mean_;}
        result_type stddev() const {return stddev_;}
        result_type min() const {return min_;}
        result_type max() const {return max_;}

        friend
        bool operator==(const param_type& x, const param_type& y)
        {return x.mean_ == y.mean_ && x.stddev_ == y.stddev_ && x.min_ == y.min_ && x.max_ == y.max_;}
        friend
        bool operator!=(const param_type& x, const param_type& y)
        {return !(x == y);}
    };

private:
    param_type p_;

public:
    // constructors and reset functions
    explicit truncated_normal_distribution(result_type mean = 0, result_type stddev = 1,
            result_type min = -std::numeric_limits<_RealType>::infinity(),
            result_type max = std::numeric_limits<_RealType>::infinity())
            : p_(param_type(mean, stddev, min, max)) {}
    explicit truncated_normal_distribution(const param_type& p)
            : p_(p){}
    void reset() {}

    // generating functions
    template<class _URNG>
    result_type operator()(_URNG& g)
    {return (*this)(g, p_);}
    template<class _URNG> result_type operator()(_URNG& g, const param_type& p);

    // property functions
    result_type mean() const {return p_.mean();}
    result_type stddev() const {return p_.stddev();}

    param_type param() const {return p_;}
    void param(const param_type& p) {p_ = p;}

    result_type min() const {return p_.min();}
    result_type max() const {return p_.max();}

    friend
    bool operator==(const truncated_normal_distribution& x,
                    const truncated_normal_distribution& y)
    {return x.p_ == y.p_;}
    friend
    bool operator!=(const truncated_normal_distribution& x,
                    const truncated_normal_distribution& y)
    {return !(x == y);}

    template <class _CharT, class _Traits, class _RT>
    friend
    std::basic_ostream<_CharT, _Traits>&
    operator<<(std::basic_ostream<_CharT, _Traits>& os,
               const truncated_normal_distribution<_RT>& x);

    template <class _CharT, class _Traits, class _RT>
    friend
    std::basic_istream<_CharT, _Traits>&
    operator>>(std::basic_istream<_CharT, _Traits>& is,
               truncated_normal_distribution<_RT>& x);
};

template <class _RealType>
template<class _URNG>
_RealType
truncated_normal_distribution<_RealType>::operator()(_URNG& g, const param_type& p)
{
    std::normal_distribution<result_type> Normal(p.mean(), p.stddev());

    std::cout << p.min() << ", " << p.max() << std::endl;
    result_type v;

    do v = Normal(g);
    while (!(v > p.min() && v < p.max()));

    return v;
}


#endif //SAMPP_TRUNCATED_RANDOM_H
