//
// Created by Amir Masoud Abdol on 2019-03-03.
//

#ifndef SAMPP_GROUPDATA_H
#define SAMPP_GROUPDATA_H

#include <armadillo>

namespace sam {

class TestStrategy;

enum class GroupType {
    Control,
    Treatment
};

class GroupData {
    
    int id;
    GroupType gtype;
    
    /// --- Descriptive statistics
    int nobs_ {0};
    double mean_ {0};
    double var_ {0};
    double stddev_ {0};
    double ses_ {0};
    
    bool is_stats_updated_ {false};
    
    /// --- Test statistics
    double pvalue_ {0};
    double effect_ {0};
    bool sig_ {false};
    
    bool is_test_stats_updated_ {false};
        
    //! Measurements
    arma::Row<double> data_;
    bool is_measurements_initd_ {false};
    
public:
    
    GroupData(int n): nobs_{n} {
        data_.resize(n);
    };
    
    GroupData(arma::Row<double> &data) : data_{data} {
        nobs_ = data.size();
        mean_ = arma::mean(data);
        var_ = arma::var(data);
        stddev_ = arma::stddev(data);
        ses_ = sqrt( var_ / nobs_ );
    };
    
    double& operator[](int i){
        return data_.at(i);
    };
    
    template <typename OStream>
    friend OStream &operator<<(OStream &os, const GroupData &data) {
        os << "nobs" << data.nobs_ <<
                "mean" << data.mean_ <<
                "var" << data.var_ <<
                "stddev" << data.stddev_ <<
                "ses" << data.ses_ <<
                "pvalue" << data.pvalue_ <<
                "effect" << data.effect_ <<
                "sig" << data.sig_;

        return os;
    }
    
    void updateStats();
    
    void testAgaist(GroupData &other_group, TestStrategy &test_strategy);
};

}

#endif //SAMPP_GROUPDATA_H
