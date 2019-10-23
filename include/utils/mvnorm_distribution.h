//
// Created by Amir Masoud Abdol on 2019-06-14.
//

#ifndef SAMPP_MVNORM_DISTRIBUTION_H
#define SAMPP_MVNORM_DISTRIBUTION_H

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
        
        explicit param_type() {
            arma::mat means(1, 1, arma::fill::zeros);
            arma::mat covs(1, 1, arma::fill::eye);
            
            dims_ = means.n_elem;
            means_ = means;
            covs_ = covs;
        }

        explicit param_type(result_type means, result_type covs)
                : dims_(means.n_elem), means_(means), covs_(covs) {
                    
            if (!means.is_colvec())
                throw std::logic_error("Mean should be a column vector.");

            if (covs.n_rows != dims_ )
                throw std::length_error("Covariance matrix has the wrong dimension.");

            if (!covs.is_symmetric() || !covs.is_square())
                throw std::logic_error("Covarinace matrix is not square or symmetrical.");
        }
        
        // TODO: I think I need a copy assignment operator for handling the sizes and special cases

        size_t dims() const {return dims_;}
        result_type means() const {return means_;}
        result_type covs() const {return covs_;}
        arma::vec covs_diag() const {return covs_.diag();}
        bool is_covs_diagmat() const {return covs_.is_diagmat();}

        friend
        bool operator==(const param_type& x, const param_type& y)
        {return arma::approx_equal(x.means_, y.means_, "absdiff", 0.001)
                && arma::approx_equal(x.covs_, y.covs_, "absdiff", 0.001); }
        friend
        bool operator!=(const param_type& x, const param_type& y)
        {return !(x == y); }
    };

private:

    arma::mat covs_lower;
    arma::mat inv_covs_lower;
    arma::mat inv_covs;
    std::normal_distribution<> norm;  // N~(0, 1)

    param_type p_;
    result_type tmp_;

public:
    
    explicit mvnorm_distribution() : p_(param_type{}) {} ;
    
    // constructor and reset functions
    explicit mvnorm_distribution(result_type means, result_type covs)
            : p_(param_type(means, covs))  {
        
        // TODO: check if it's diagonal, initiate the diag model

        tmp_.resize(p_.dims(), 1);

        if (!p_.is_covs_diagmat())
            factorize_covariance();
    }

    explicit mvnorm_distribution(const param_type& p)
            : p_(p) {}
    void reset() { norm.reset(); };

    // generating functions
    template<class URNG>
    result_type operator()(URNG& g)
    {return (*this)(g, p_);}

    template<class URNG>
    result_type operator()(URNG& g, const param_type& parm);

    // property functions
    result_type means() const {return p_.means();}
    result_type covs() const {return p_.covs();}

    param_type param() const {return p_;}
    void param(const param_type& params) {
        // TODO: This needs more checks.
        p_ = params;
        
        tmp_.resize(p_.dims());
        
        if (!p_.is_covs_diagmat())
            factorize_covariance();
    }

    void factorize_covariance() {
        covs_lower = arma::chol(p_.covs(), "lower");
        inv_covs_lower = arma::inv(arma::trimatl(covs_lower));
        inv_covs = inv_covs_lower.t() * inv_covs_lower;
    }

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
    
    tmp_.imbue( [&]() { return norm(g); } );
    if (parm.is_covs_diagmat()){
        return arma::sqrt(parm.covs_diag()) % tmp_ + parm.means();
    }else{
        return covs_lower * tmp_ + parm.means();
    }
    
}

// Truncated Normal Distribution
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
            
            dims_ = means.n_elem;

            // Checking whether dimensions matches
            if (lowers_.n_rows != dims_ || uppers_.n_rows != dims_)
                throw std::length_error("Check your arrays size");

            if (!covs.is_symmetric() || !covs.is_square())
                throw std::logic_error("Covariance matrix is not symmetric.");
            
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
    
    mvnorm_distribution<> mvnorm{};   // MN ~ ((0), ((1)))
    
    param_type p_;
    result_type tmp_;

    int res_;
    
public:
    // constructor and reset functions
    explicit truncated_mvnorm_distribution(result_type means, result_type covs, result_type lowers, result_type uppers)
    : p_(param_type(means, covs, lowers, uppers)) {
        
        mvnorm.param(mvnorm_distribution<>::param_type{means, covs});
    }
    
    explicit truncated_mvnorm_distribution(const param_type& p)
    : p_(p) {}
    void reset() {mvnorm.reset();};
    
    // generating functions
    template<class URNG>
    result_type operator()(URNG& g)
    {return (*this)(g, p_);}
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

// Implementation of rejection algorithm
template <class _RealType>
template <class _URNG>
truncated_mvnorm_distribution<double>::result_type
truncated_mvnorm_distribution<_RealType>::operator()(_URNG &g, const truncated_mvnorm_distribution<_RealType>::param_type &parm) {
    
    do {
        tmp_ = mvnorm(g);
        res_ = arma::accu((tmp_ < p_.lowers()) + (tmp_ > p_.uppers()));
    }while(res_ != 0);
    
    return tmp_;
}

#endif //SAMPP_MVNORM_DISTRIBUTION_H
