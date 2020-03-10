//
// Created by Amir Masoud Abdol on 2019-03-03.
//

#ifndef SAMPP_GROUPDATA_H
#define SAMPP_GROUPDATA_H

#include <string>

#include <armadillo>

namespace sam {

class TestStrategy;
enum class HackingMethod;

enum class GroupType {
    Control,
    Treatment
};

class GroupData {
    
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
    double ses_ {0};
    
    bool is_stats_updated_ {false};
    
    /// --- Test statistics
    /// ... These can be their own type, and determined by the TestStrategies
    double stats_ {0};
    double pvalue_ {0};
    double effect_ {0};
    bool sig_ {false};
    bool is_test_stats_updated_ {false};
        
    //! Measurements
    arma::Row<double> measurements_;
    bool is_measurements_initd_ {false};
    
    /// --- Hacking Meta
    bool is_hacked_ {false};
    std::vector<HackingMethod> hacking_history_;
    int n_added_obs {0};
    int n_removed_obs {0};
    
//public:
    
    GroupData() {};
    
    GroupData(int id_, GroupType type_) : id_{id_}, gtype{type_} {};
    
    GroupData(int n): nobs_{n} {
        measurements_.resize(n);
    };
    
    GroupData(arma::Row<double> &data) : measurements_{data} {
        updateStats();
    };
    
    /// Getter / Setter
    
    arma::Row<double> measurements() const { return measurements_; };
    void set_measurements(arma::Row<double> &meas) {
        measurements_ = meas;
        is_stats_updated_ = false;
    }
    
    template <typename OStream>
    friend OStream &operator<<(OStream &os, const GroupData &data) {
        os <<   "id: " << data.id_ <<
                " nobs: " << data.nobs_ <<
                " mean: " << data.mean_ <<
                " var: " << data.var_ <<
                " stddev: " << data.stddev_ <<
                " ses: " << data.ses_ <<
                " stats: " << data.stats_ <<
                " pvalue: " << data.pvalue_ <<
                " effect: " << data.effect_ <<
                " sig: " << data.sig_;

        return os;
    }
    
    operator std::map<std::string, std::string>() {
        
        std::map<std::string, std::string> out_map;
        
        out_map["gid"] = std::to_string(id_);
        
        out_map["nobs"] = std::to_string(nobs_);
        out_map["mean"] = std::to_string(mean_);
        out_map["var"] = std::to_string(var_);
        out_map["stddev"] = std::to_string(stddev_);
        out_map["ses"] = std::to_string(ses_);
        
        /// This can be replaced with an map.insert() and basically
        /// just insert TestStrategy's map() operator
        out_map["pvalue"] = std::to_string(pvalue_);
        out_map["effect"] = std::to_string(effect_);
        out_map["sig"] = std::to_string(sig_);
        
        out_map["is_hacked"] = std::to_string(is_hacked_);
        
        return std::map<std::string, std::string>();
    };
 
    void updateStats();
    
    void testAgaist(GroupData &other_group, TestStrategy &test_strategy);
};

}

#endif //SAMPP_GROUPDATA_H
