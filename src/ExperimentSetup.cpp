//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <stdexcept>
#include <armadillo>

#include "ExperimentSetup.h"

using namespace sam;

std::ostream& operator<<(std::ostream& os, ExperimentType et)
{
    switch(et)
    {
        case ExperimentType::LinearModel:
            os << "Fixed Model";
            break;
        case ExperimentType::LatentModel:
            os << "Latent Model";
            break;
        default:
            os.setstate(std::ios_base::failbit);
    }
    return os;
}


void ExperimentSetup::setValueOf(std::string pname, double val) {
    std::fill(true_parameters_[pname].begin(),
              true_parameters_[pname].end(),
              val);
};

void ExperimentSetup::setValueOf(std::string pname, arma::Mat<double>& val_v) {
    true_parameters_[pname] = val_v;
};

void ExperimentSetup::setValueOf(std::string pname, int min, int max) {
    
    true_parameters_[pname].imbue( [this, min, max]() { return rng_stream->uniform(min, max); } );
    
};

void ExperimentSetup::setValueOf(std::string pname, std::function<double(void)> fun) {
    
    true_parameters_[pname].imbue( [fun](){return fun(); });
    
};


ExperimentSetup::ExperimentSetup(json& config) {
    
    // Setting the seed for number of observation
    rng_stream = new RandomNumberGenerator(rand());
    
        
    if (config["data-strategy"].is_null()){
        throw std::invalid_argument("The Data Strategy parameter is not provided. Check the documentation for more info.");
    }else if (stringToExperimentType.find(config["data-strategy"]) == stringToExperimentType.end()){
        throw std::invalid_argument("Unknown Data Strategy.");
    }
    experiment_type = stringToExperimentType.find(config["data-strategy"])->second;
    
    nc_ = config["n-conditions"];
    nd_ = config["n-dep-vars"];
    ni_ = config["n-items"];
    ng_ = nc_ * nd_;
    nrows_ = ng_ * ni_;
    
    initializeMemory();
    
    // true_parameters_["nobs"].resize(ng_);
    if (config["n-obs"].is_array() && config["n-obs"][0].is_array()){
        intervals = config["n-obs"][0].get<std::vector<double>>();
        weights = config["n-obs"][1].get<std::vector<double>>();
        int nobs = rng_stream->genSampleSize(intervals, weights);
        std::fill(true_parameters_["nobs"].begin(), true_parameters_["nobs"].end(), nobs);
        is_n_randomized = true;
//    }else if (config["n-obs"] == "random"){
//        isNRandomized = true;
//        int nobs = RNGEngine->genSampleSize(0.75, 20, 100, 300);
//        std::fill(true_parameters_["nob"].begin(), true_parameters_["nob"].end(), nobs);
//
//        config["n-obs"] = nobs;
    }else{
        if (config["n-obs"].is_array()){
            if (config["n-obs"].size() != ng_){
                throw std::invalid_argument( "Size of --n-obs does not match the size of the experiment.");
            }
            true_parameters_["nobs"] = config["n-obs"].get<std::vector<double>>();
        }else if (config["n-obs"].is_number()){
            // Broadcase the given --n-obs to a vector of length `ng`
            true_parameters_["nobs"] = std::vector<double>(ng_, config["n-obs"]);
        }
    }

    
    if (config["means"].is_array()){
        if (config["means"].size() != ng_){
            throw std::invalid_argument( "Size of --means does not match the size of the experiment.");
        }
        true_parameters_["means"] = config["means"].get<std::vector<double>>();
    }else if (config["means"].is_number()){
        // Broadcase the given --means to a vector of length `ng`
        true_parameters_["means"] = std::vector<double>(ng_, config["means"]);
    }else{
        throw std::invalid_argument("means is invalid or not provided.");
    }

    std::cout << true_parameters_["means"] << std::endl;
    
    if (config["vars"].is_array()){
        if (config["vars"].size() != ng_){
            throw std::invalid_argument( "Size of --vars does not match the size of the experiment.");
        }
        true_parameters_["vars"] = config["vars"].get<std::vector<double>>();
    }else if (config["vars"].is_number()){
        // Broadcast the given --vars to a vector of length `ng`
        true_parameters_["vars"] = std::vector<double>(ng_, config["vars"]);
    }else{
        throw std::invalid_argument("vars is invalid or not provided.");
    }
    // std::cout << true_parameters_["var"];
    

    
    if (config["covs"].is_array()){
        if (config["covs"].size() != ng_){
            throw std::invalid_argument( "Size of --covs does not match the size of the experiment.");
        }
        // DOC: Notify the user that the `sds` will be discarded.
        auto sigma = config["covs"].get<std::vector<std::vector<double>>>();
        for (int i = 0; i < sigma.size(); i++) {
            true_parameters_["sigma"].row(i) = arma::rowvec(sigma[i]);
        }
    }else if (config["covs"].is_number()){
        // Broadcase the --covs to the a matrix, and replace the diagonal values with
        // the value already given by --vars.
        double cov = config["covs"];

        true_parameters_["sigma"].zeros(ng_, ng_);
        true_parameters_["sigma"].fill(cov);
        true_parameters_["sigma"].diag() = true_parameters_["vars"];
    }else{
        throw std::invalid_argument("covs is invalid or not provided.");
    }
//    }
    // std::cout << true_parameters_["sigm"] << std::endl;

    // Factor Loading ...
    // CHECK: I think there are `ni` of these,
    // CHECK: I think they should be `nrows`
    if (config["loadings"].is_array()){
        if (config["loadings"].size() != ni_){
            throw std::invalid_argument( "Size of --loadings does not match the size of the experiment.");
        }
        true_parameters_["loadings"] = config["loadings"].get<std::vector<double>>();
    }else if (config["loadings"].is_number()){
        // Broadcast the given --loadings to a vector of length `ng`
        true_parameters_["loadings"] = std::vector<double>(ni_, config["loadings"]);
    }else{
        throw std::invalid_argument("loadings is invalid or not provided.");
    }
    
    // TODO: Maybe add me to the config file
    // true_parameters["error_means"].resize(nrows_);
    true_parameters_["error_means"].fill(0);
    
    // Error's Standard Deviations
    if (config["err-vars"].is_array()){
        if (config["err-vars"].size() != nrows_){
            throw std::invalid_argument( "Size of --err-vars does not match the size of the experiment.");
        }
        true_parameters_["error_vars"] = config["err-vars"].get<std::vector<double>>();
    }else if (config["err-vars"].is_number()){
        // Broadcast the given --err-vars to a vector of length `nrows`
        true_parameters_["error_vars"] = std::vector<double>(nrows_, config["err-vars"]);
    }else{
        throw std::invalid_argument("err-vars is invalid or not provided.");
    }
    // std::cout << errorVars << std::endl;
    
    // Error's Covariant Matrix
    if (config["err-covs"].is_array()){
        if (config["err-covs"].size() != nrows_){
            throw std::invalid_argument( "Size of --err-covs does not match the size of the experiment.");
        }
        auto covs = config["err-covs"].get<std::vector<std::vector<double>>>();
        for (int i = 0; i < covs.size(); i++) {
            true_parameters_["error_cov"].row(i) = arma::rowvec(covs[i]);
        }
        
    }else if (config["err-covs"].is_number()){
        // Broadcase the --err-covs to the a matrix, and replace the diagonal values with
        // the value already given by --vars.
        double cov = config["err-covs"];
//        for (int r = 0; r < nrows; r++) {
//            errorCov.push_back(std::vector<double>(nrows, cov));
//            errorCov[r][r] = errorVars[r];
//        }
        true_parameters_["error_cov"].zeros(nrows_, nrows_);
        true_parameters_["error_cov"].fill(cov);
        true_parameters_["error_cov"].diag() = true_parameters_["error_vars"];
    }else{
        throw std::invalid_argument("err-covs is invalid or not provided.");
    }
    // std::cout << errorCov << std::endl;
    
}

void ExperimentSetup::randomize_nObs() {
//    nobs = RNGEngine->genSampleSize(0.75, 20, 100, 300);
    int n = rng_stream->genSampleSize(intervals, weights);
    std::fill(true_parameters_["nobs"].begin(), true_parameters_["nobs"].end(), n);
}

