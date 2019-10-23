//
// Created by Amir Masoud Abdol on 23/10/2019.
//

#ifndef SAMPP_TRUNCATED_MVNORM_DISTRIBUTION_H
#define SAMPP_TRUNCATED_MVNORM_DISTRIBUTION_H


#include <random>
#include <iostream>
#include "boost/math/distributions/normal.hpp"

using boost::math::normal;

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

        size_t dims() const {return dims_;}
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

    std::uniform_real_distribution<> uniform{};
    param_type p_;

    arma::mat sub1(arma::mat x, int i) {
        x.shed_col(i);
        x.shed_row(i);
        return x;
    }

    arma::mat sub2(arma::mat x, int a, int b){
        x.shed_col(b);
        return(x.row(a));
    }

    arma::vec negSubCol(arma::vec x, int i){
        x.shed_row(i);
        return(x);
    }

    arma::rowvec negSubRow(arma::rowvec x, int i){
        x.shed_col(i);
        return(x);
    }


public:
    // constructor and reset functions
    explicit truncated_mvnorm_distribution(result_type means, result_type covs, result_type lowers, result_type uppers)
            : p_(param_type(means, covs, lowers, uppers)) {

    }

    explicit truncated_mvnorm_distribution(const param_type& p)
            : p_(p) {}
    void reset() {uniform.reset();};

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

    int n {5};
    int d = parm.dims();
    arma::mat trace = arma::zeros(n, d); // trace of MCMC chain

    // draw from U(0,1)
    arma::vec U(n*d);
    U.imbue( [&](){ return uniform(g); });

    int l = 0; // iterator for U

    // calculate conditional standard deviations
    arma::vec sd(d);
    arma::cube P = arma::zeros(1,d - 1, d);

    for(int i=0; i<d; i++){
        //partitioning of sigma
        arma::mat Sigma = sub1(parm.covs(), i);
        double sigma_ii = parm.covs()(i, i);
        arma::rowvec Sigma_i = sub2(parm.covs(), i, i);

        P.slice(i) = Sigma_i * Sigma.i();
        double p_i = arma::as_scalar(P.slice(i) * Sigma_i.t());
        sd(i) = sqrt(sigma_ii - p_i);
    }

    arma::vec x = parm.means();

    //run Gibbs sampler for specified chain length (MCMC chain of n samples)
    for(int j=0; j<n; j++){

        //sample all conditional distributions
        for(int i=0; i<d; i++){

            //calculation of conditional expectation and conditional variance
            arma::rowvec slice_i = P.slice(i);
            arma::vec slice_i_times = slice_i * (negSubCol(x,i) - negSubCol(x,i));
            double slice_i_times_double = arma::as_scalar(slice_i_times);
            double mu_i = parm.means()(i) + slice_i_times_double;

            //transformation
            double Fa = cdf(normal{mu_i, sd(i)}, parm.lowers()(i));
            double Fb = cdf(normal{mu_i, sd(i)}, parm.uppers()(i));

            x(i) = mu_i + sd(i) * quantile(normal{0, 1}, U(l) * (Fb - Fa) + Fa);

            l = l + 1;

        }

        trace.row(j) = x.t();

    }

    return trace.t();

}

#endif //SAMPP_TRUNCATED_MVNORM_DISTRIBUTION_H
