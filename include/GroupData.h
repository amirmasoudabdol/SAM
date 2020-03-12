//
// Created by Amir Masoud Abdol on 2019-03-03.
//

#ifndef SAMPP_GROUPDATA_H
#define SAMPP_GROUPDATA_H

#include <string>

#include <armadillo>

namespace sam {

class TestStrategy;
class EffectStrategy;
enum class HackingMethod;

enum class GroupType {
    Control,
    Treatment,
    
    //! Indicating that the group contains pooled data from other groups
    Pooled
};

class GroupData {
    
    //! Measurements
    arma::Row<double> measurements_;
    bool is_measurements_initd_ {false};
    
public: // Public for now
    
    int id_;
    GroupType gtype;
    
    /// --- Distribution/Population Parameters
    /// These can be their own type, and determined by the DataStrategies
    double true_mean_ {0};
    double true_std_ {0};
    double true_nobs_ {0};
//    Distribution dist;
//    json dist_params;
    
    /// --- Descriptive statistics
    int nobs_ {0};
    double mean_ {0};
    double var_ {0};
    double stddev_ {0};
    double sei_ {0};
    
    bool is_stats_updated_ {false};
    
    /// --- Test statistics
    /// ... These can be their own type, and determined by the TestStrategies
    double stats_ {0};
    double pvalue_ {0};
    double effect_ {0};
    int eff_side_ {0};
    bool sig_ {false};
    bool is_test_stats_updated_ {false};
    
    /// --- Hacking Meta
    bool is_hacked_ {false};
    std::vector<HackingMethod> hacking_history_;
    int n_added_obs {0};
    int n_removed_obs {0};
    
    GroupData() {};
    
    GroupData(int id_, GroupType type_) : id_{id_}, gtype{type_} {};
    
    GroupData(arma::Row<double> &data) : measurements_{data} {
        updateStats();
    };
    
    /// Getter / Setter
    
    arma::Row<double>& measurements() { return measurements_; };
    const arma::Row<double>& measurements() const {return measurements_; };
    
    void set_measurements(const arma::Row<double> meas) {
        measurements_ = meas;
        nobs_ = meas.size();
        is_stats_updated_ = false;
    }
    
    void add_measurements(const arma::Row<double> new_meas) {
        measurements_.insert_cols(nobs_, new_meas);
        is_stats_updated_ = false;
    }
    
    void del_measurements(const arma::uvec &idxs) {
        measurements_.shed_cols(idxs);
        is_stats_updated_ = false;
    }
    
    template <typename OStream>
    friend OStream &operator<<(OStream &os, const GroupData &data) {
        os <<   "id: " << data.id_ <<
                " nobs: " << data.nobs_ <<
                " mean: " << data.mean_ <<
                " var: " << data.var_ <<
                " stddev: " << data.stddev_ <<
                " sei: " << data.sei_ <<
                " stats: " << data.stats_ <<
                " pvalue: " << data.pvalue_ <<
                " effect: " << data.effect_ <<
                " sig: " << data.sig_;

        return os;
    }
    
    operator std::map<std::string, std::string>();
 
    void updateStats();
    
    void testAgaist(const GroupData &other_group, TestStrategy &test_strategy);
    
    void effectComparedTo(const GroupData &other_group, EffectStrategy &effect_strategy);
    
};

}

#endif //SAMPP_GROUPDATA_H
