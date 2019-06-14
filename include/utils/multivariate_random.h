//
// Created by Amir Masoud Abdol on 2019-06-14.
//

#ifndef SAMPP_MULTIVARIATE_RANDOM_H
#define SAMPP_MULTIVARIATE_RANDOM_H

#include <random>
#include <iostream>

template <class _RealType = double>
class mvnorm_distribution
{
public:
    // types
    typedef arma::mat result_type;

    class param_type
    {
        size_t dims_;
        result_type means_;
        result_type covs_;
    public:
        typedef mvnorm_distribution distribution_type;

        explicit param_type(result_type means, result_type covs)
                : means_(means), covs_(covs) {
            dims_ = means.n_rows;
        }

        size_t ndims() const {return dims_;}
        result_type means() const {return means_;}
        result_type covs() const {return covs_;}

        friend
        bool operator==(const param_type& x, const param_type& y)
        {return arma::approx_equal(x.means_, y.means_, "absdiff", 0.001)
                && arma::approx_equal(x.covs_, y.covs_, "absdiff", 0.001); }
        friend
        bool operator!=(const param_type& x, const param_type& y)
        {return !(x == y); }
    };

private:
    arma::gmm_full gf_model_;
    arma::gmm_diag gd_model_;
    param_type p_;
    result_type v_;
    int i_ = -1;
    int seed_ = 42;

public:
    // constructor and reset functions
    explicit mvnorm_distribution(result_type means, result_type covs)
            : p_(param_type(means, covs)) {
        // check if it's diagonal, initiate the diag model
        gf_model_.reset(p_.ndims(), 1);

        // I need to redesign the params...
        arma::mat m(p_.ndims(), 1);
        arma::cube c(p_.ndims(), p_.ndims(), 1);
        m.col(0) = p_.means();
        c.slice(0) = p_.covs();

        //        gf_model_.set_means(p_.means());
        //        gf_model_.set_fcovs(p_.covs());

        gf_model_.set_means(m);
        gf_model_.set_fcovs(c);

        v_.set_size(p_.ndims());
    }

    explicit mvnorm_distribution(const param_type& p)
            : p_(p) {}
    void reset() {};

    // seeding
    int seed() const {return seed_;}
    void seed(int s) {seed_ = s; arma::arma_rng::set_seed(seed_);}

    // generating functions
    template<class URNG>
    result_type operator()(URNG& g)
    {return (*this)(g, p_);}
//    template<class URNG>
//    result_type operator()(URNG& g, int N);
    template<class URNG>
    result_type operator()(URNG& g, const param_type& parm);


    // property functions
    result_type means() const {return p_.means();}
    result_type covs() const {return p_.covs();}

    param_type param() const {return p_;};
    void param(const param_type& params) { p_ = params;}

    result_type min() const {return -std::numeric_limits<_RealType>::infinity();}
    result_type max() const {return std::numeric_limits<_RealType>::infinity();}

    friend bool operator==(const mvnorm_distribution& x,
                           const mvnorm_distribution& y)
    {return x.p_ == y.p_;}
    friend bool operator!=(const mvnorm_distribution& x,
                           const mvnorm_distribution& y)
    {return !(x == y);}

    template <class charT, class traits>
    friend
    std::basic_ostream<charT, traits>&
    operator<<(std::basic_ostream<charT, traits>& os,
               const mvnorm_distribution& means);

    template <class charT, class traits>
    friend
    std::basic_istream<charT, traits>&
    operator>>(std::basic_istream<charT, traits>& is,
               mvnorm_distribution& means);

};

template <class _RealType>
template <class _URNG>
mvnorm_distribution<double>::result_type
mvnorm_distribution<_RealType>::operator()(_URNG &g, const mvnorm_distribution<_RealType>::param_type &parm) {
    return gf_model_.generate();
}

//template <class _RealType>
//template <class _URNG>
//mvnorm_distribution<double>::result_type
//mvnorm_distribution<_RealType>::operator()(_URNG &g, const int N) {
//    return gf_model_.generate(N);
//}


template <class _RealType = double>
class truncated_mvnorm_distribution
{
public:
    // types
    typedef arma::mat result_type;
    
    class param_type
    {
        size_t dims_;
        result_type means_;
        result_type covs_;
        result_type lowers_;
        result_type uppers_;
        
    public:
        typedef truncated_mvnorm_distribution distribution_type;
        
        explicit param_type(result_type means, result_type covs, result_type lowers, result_type uppers)
        : means_(means), covs_(covs), lowers_(lowers), uppers_(uppers) {
            dims_ = means.n_rows;
            // TODO: Expand the test
            if (lowers_.size() != uppers_.size())
                throw std::length_error("Check your arrays size");
        }
        
        size_t ndims() const {return dims_;}
        result_type means() const {return means_;}
        result_type covs() const {return covs_;}
        result_type lowers() const {return lowers_;}
        result_type uppers() const {return uppers_;}
        
        friend
        bool operator==(const param_type& x, const param_type& y)
        {return arma::approx_equal(x.means_, y.means_, "absdiff", 0.001)
            && arma::approx_equal(x.covs_, y.covs_, "absdiff", 0.001)
            && arma::approx_equal(x.lowers_, y.lowers_, "absdiff", 0.001)
            && arma::approx_equal(x.uppers_, y.uppers_, "absdiff", 0.001); }
        friend
        bool operator!=(const param_type& x, const param_type& y)
        {return !(x == y); }
    };
    
private:
    arma::gmm_full gf_model_;
    arma::gmm_diag gd_model_;
    param_type p_;
    result_type v_;
    int i_ = -1;
    int seed_ = 42;
    
public:
    // constructor and reset functions
    explicit truncated_mvnorm_distribution(result_type means, result_type covs, result_type lowers, result_type uppers)
    : p_(param_type(means, covs, lowers, uppers)) {
        // check if it's diagonal, initiate the diag model
        gf_model_.reset(p_.ndims(), 1);
        
        // I need to redesign the params...
        arma::mat m(p_.ndims(), 1);
        arma::cube c(p_.ndims(), p_.ndims(), 1);
        m.col(0) = p_.means();
        c.slice(0) = p_.covs();
        
        //        gf_model_.set_means(p_.means());
        //        gf_model_.set_fcovs(p_.covs());
        
        gf_model_.set_means(m);
        gf_model_.set_fcovs(c);
        
        v_.set_size(p_.ndims());
    }
    
    explicit truncated_mvnorm_distribution(const param_type& p)
    : p_(p) {}
    void reset() {};
    
    // seeding
    int seed() const {return seed_;}
    void seed(int s) {seed_ = s; arma::arma_rng::set_seed(seed_);}
    
    // generating functions
    template<class URNG>
    result_type operator()(URNG& g)
    {return (*this)(g, p_);}
    //    template<class URNG>
    //    result_type operator()(URNG& g, int N);
    template<class URNG>
    result_type operator()(URNG& g, const param_type& parm);
    
    
    // property functions
    result_type means() const {return p_.means();}
    result_type covs() const {return p_.covs();}
    
    param_type param() const {return p_;};
    void param(const param_type& params) { p_ = params;}
    
    result_type min() const {return p_.lowers();}
    result_type max() const {return p_.uppers();}
    
    friend bool operator==(const truncated_mvnorm_distribution& x,
                           const truncated_mvnorm_distribution& y)
    {return x.p_ == y.p_;}
    friend bool operator!=(const truncated_mvnorm_distribution& x,
                           const truncated_mvnorm_distribution& y)
    {return !(x == y);}
    
    template <class charT, class traits>
    friend
    std::basic_ostream<charT, traits>&
    operator<<(std::basic_ostream<charT, traits>& os,
               const truncated_mvnorm_distribution& means);
    
    template <class charT, class traits>
    friend
    std::basic_istream<charT, traits>&
    operator>>(std::basic_istream<charT, traits>& is,
               truncated_mvnorm_distribution& means);
    
};

template <class _RealType>
template <class _URNG>
truncated_mvnorm_distribution<double>::result_type
truncated_mvnorm_distribution<_RealType>::operator()(_URNG &g, const truncated_mvnorm_distribution<_RealType>::param_type &parm) {
    static int res_;
    do {
        v_ = gf_model_.generate();
        res_ = arma::accu((v_ < p_.lowers()) + (v_ > p_.uppers()));
    } while(res_ != 0);
    
    return v_;
}

#endif //SAMPP_MULTIVARIATE_RANDOM_H
