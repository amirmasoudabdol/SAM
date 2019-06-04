//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <stdexcept>
#include <tuple>

#include <utils/magic_enum.hpp>

#include "ExperimentSetup.h"
#include "Utilities.h"

using namespace sam;

ExperimentSetupBuilder ExperimentSetup::create() {
    return ExperimentSetupBuilder();
}

ExperimentSetup::ExperimentSetup(json& config) {
    
    using namespace magic_enum;
    
    // Setting the seed for number of observation
    rng_stream = new RandomNumberGenerator(rand());
    
    auto data_model =  enum_cast<DataStrategy::DataModel>(config["data-strategy"].get<std::string>());
    if (data_model.has_value()) {
        dsp_.name = data_model.value();
    }
    
    nc_ = config["n-conditions"];
    nd_ = config["n-dep-vars"];
    ni_ = config["n-items"];
    ng_ = nc_ * nd_;
    nrows_ = ng_ * ni_;

    auto nobs_t = get_expr_setup_params<int>(config["n-obs"], ng_);
    nobs_ = std::get<0>(nobs_t);
    params_dist["n-obs"] = std::get<1>(nobs_t);

    auto means_t = get_expr_setup_params<double>(config["means"], ng_);
    means_ = std::get<0>(means_t);
    params_dist["means"] = std::get<1>(means_t);

    auto vars_t = get_expr_setup_params<double>(config["vars"], ng_);
    vars_ = std::get<0>(vars_t);
    params_dist["vars"] = std::get<1>(vars_t);
    
    auto covs_t = get_expr_setup_params<double>(config["covs"], ng_ * (ng_ - 1) / 2);
    covs_ = std::get<0>(covs_t);
    params_dist["covs"] = std::get<1>(covs_t);
    
    // Constructing the covarinace matrix based on the given covariance and variances.
    sigma_ = constructCovMatrix(vars_, covs_, ng_);

    auto loadings_t = get_expr_setup_params<double>(config["loadings"], ni_);
    loadings_ = std::get<0>(loadings_t);
    params_dist["loadings"] = std::get<1>(loadings_t);

    auto error_means_t = get_expr_setup_params<double>(config["err-means"], nrows_);
    error_means_ = std::get<0>(error_means_t);
    params_dist["err-means"] = std::get<1>(error_means_t);

    auto error_vars_t = get_expr_setup_params<double>(config["err-vars"], nrows_);
    error_vars_ = std::get<0>(error_vars_t);
    params_dist["err-vars"] = std::get<1>(error_vars_t);

    auto error_covs_t = get_expr_setup_params<double>(config["err-covs"], nrows_ * (nrows_ - 1) / 2);
    error_covs_ = std::get<0>(error_covs_t);
    params_dist["err-covs"] = std::get<1>(error_covs_t);
    
    // Constructing the covariance matrix
    error_sigma_ = constructCovMatrix(error_vars_, error_covs_, nrows_);
    
}

void ExperimentSetup::randomize_parameters() {
    if (params_dist["n-obs"]){
        fill_vector<int>(nobs_, nobs_.size(), params_dist["n-obs"](gen));
    }

    if (params_dist["means"]){
        fill_vector<double>(means_, means_.size(), params_dist["means"](gen));
    }

    if (params_dist["vars"]){
        fill_vector<double>(vars_, vars_.size(), params_dist["vars"](gen));
    }

    if (params_dist["covs"]){
        fill_vector<double>(covs_, covs_.size(), params_dist["vars"](gen));
    }

    if (params_dist["vars"] || params_dist["covs"])
        sigma_ = constructCovMatrix(vars_, covs_, ng_);

    // TODO: TEST US!
    // if (params_dist["loadings"]){
    //     fill_vector<double>(loadings_, loadings_.size(), params_dist["loadings"](gen));
    // }

    // if (params_dist["err-means"]){
    //     fill_vector<double>(error_means_, error_means_.size(), params_dist["err-means"](gen));
    // }

    // if (params_dist["err-vars"]){
    //     fill_vector<double>(error_vars_, error_vars_.size(), params_dist["err-vars"](gen));
    // }

}



ExperimentSetupBuilder& ExperimentSetupBuilder::fromConfigFile(json &config) {

    // Setting the seed for number of observation
    setup.rng_stream = new RandomNumberGenerator(rand());
    
    setup.dsp_.name = config["data-strategy"];
    
    setup.nc_ = config["n-conditions"];
    setup.nd_ = config["n-dep-vars"];
    setup.ni_ = config["n-items"];
    setup.ng_ = setup.nc_ * setup.nd_;
    setup.nrows_ = setup.ng_ * setup.ni_;
    
    auto nobs_t = get_expr_setup_params<int>(config["n-obs"], setup.ng_);
    setup.nobs_ = std::get<0>(nobs_t);
    setup.params_dist["n-obs"] = std::get<1>(nobs_t);

    auto means_t = get_expr_setup_params<double>(config["means"], setup.ng_);
    setup.means_ = std::get<0>(means_t);
    setup.params_dist["means"] = std::get<1>(means_t);

    auto vars_t = get_expr_setup_params<double>(config["vars"], setup.ng_);
    setup.vars_ = std::get<0>(vars_t);
    setup.params_dist["vars"] = std::get<1>(vars_t);

    auto loadings_t = get_expr_setup_params<double>(config["loadings"], setup.ni_);
    setup.loadings_ = std::get<0>(loadings_t);
    setup.params_dist["loadings"] = std::get<1>(loadings_t);

    auto error_means_t = get_expr_setup_params<double>(config["err-means"], setup.nrows_);
    setup.error_means_ = std::get<0>(error_means_t);
    setup.params_dist["err-means"] = std::get<1>(error_means_t);

    auto error_vars_t = get_expr_setup_params<double>(config["err-vars"], setup.nrows_);
    setup.error_vars_ = std::get<0>(error_vars_t);
    setup.params_dist["err-vars"] = std::get<1>(error_vars_t);
    
    if (config["covs"].is_array()){
        if (config["covs"].size() != setup.ng_){
            throw std::invalid_argument( "Size of covs does not match the size of the experiment.");
        }
        // DOC: Notify the user that the `sds` will be discarded.
        auto sigma = config["covs"].get<std::vector<std::vector<double>>>();
        for (int i = 0; i < sigma.size(); i++) {
            setup.sigma_.row(i) = arma::rowvec(sigma[i]);
        }
    }else if (config["covs"].is_number()){
        // Broadcase the covs to the a matrix, and replace the diagonal values with
        // the value already given by vars.
        double cov = config["covs"];

        setup.sigma_.zeros(setup.ng_, setup.ng_);
        setup.sigma_.fill(cov);
        setup.sigma_.diag() = setup.vars_;
    }else{
        throw std::invalid_argument("covs is invalid or not provided.");
    }

    // Error's Covariant Matrix
    if (config["err-covs"].is_array()){
        if (config["err-covs"].size() != setup.nrows_){
            throw std::invalid_argument( "Size of err-covs does not match the size of the experiment.");
        }
        auto covs = config["err-covs"].get<std::vector<std::vector<double>>>();
        for (int i = 0; i < covs.size(); i++) {
            setup.error_sigma_.row(i) = arma::rowvec(covs[i]);
        }
        
    }else if (config["err-covs"].is_number()){
        // Broadcase the err-covs to the a matrix, and replace the diagonal values with
        // the value already given by vars.
        double cov = config["err-covs"];

        setup.error_sigma_.zeros(setup.nrows_, setup.nrows_);
        setup.error_sigma_.fill(cov);
        setup.error_sigma_.diag() = setup.error_vars_;
    }else{
        throw std::invalid_argument("err-covs is invalid or not provided.");
    }

    return *this;
    
}
