//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <stdexcept>
#include <tuple>

#include "ExperimentSetup.h"
#include "Utilities.h"
#include "DataStrategy.h"

using namespace sam;

ExperimentSetupBuilder ExperimentSetup::create() {
    return ExperimentSetupBuilder();
}

ExperimentSetup::ExperimentSetup(json& config) {
    
//    using namespace magic_enum;
    
    ds_name = config["data_strategy"]["_name"];
    
    nc_ = config["n_conditions"];
    nd_ = config["n_dep_vars"];
    ng_ = nc_ * nd_;

    auto nobs_t = get_expr_setup_params<int>(config["n_obs"], ng_);
    nobs_ = std::get<0>(nobs_t);
    params_dist["n_obs"] = std::get<1>(nobs_t);
    
    tsp_conf = config["test_strategy"];
    dsp_conf = config["data_strategy"];
    esp_conf = config["effect_strategy"];
    
}

// TODO: Rethink this fuction!
void ExperimentSetup::randomize_parameters() {
    
    if (params_dist["n_obs"]){
        fill_vector<int>(nobs_, nobs_.size(), Random::get(params_dist["n_obs"]));
    }

}



ExperimentSetupBuilder& ExperimentSetupBuilder::fromConfigFile(json &config) {
    
    setup.ds_name = config["data_strategy"]["_name"];
    
    setup.nc_ = config["n_conditions"];
    setup.nd_ = config["n_dep_vars"];
    setup.ng_ = setup.nc_ * setup.nd_;

    return *this;
    
}
