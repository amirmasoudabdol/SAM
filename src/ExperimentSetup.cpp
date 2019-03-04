//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <stdexcept>

#include "ExperimentSetup.h"

std::ostream& operator<<(std::ostream& os, ExperimentType et)
{
    switch(et)
    {
        case ExperimentType::LinearModel   : os << "Fixed Model" ;  break;
        case ExperimentType::LatentModel   : os << "Latent Model";  break;
        default    : os.setstate(std::ios_base::failbit);
    }
    return os;
}


ExperimentSetup::ExperimentSetup(json& config) {
    
    int metaSeed;
    if (config["--meta-seed"].is_null() || config["--meta-seed"].get<int>() == 0){
        metaSeed = rand();
    }else{
        // --meta-seed is a number
        metaSeed = config["--meta-seed"];
    }
    RNGEngine = new RandomNumberGenerator(metaSeed, false);
    
        
    if (config["--data-strategy"].is_null()){
        throw std::invalid_argument("The \"--data-strategy\" parameter is not provided. Check README.md for more info.");
    }else if (stringToExperimentType.find(config["--data-strategy"]) == stringToExperimentType.end()){
        throw std::invalid_argument("Unrecognized \"--data-strategy\". See README.md.");
    }
    experimentType = stringToExperimentType.find(config["--data-strategy"])->second;

    if (experimentType == ExperimentType::LatentModel)
        isCorrelated = true;
    
    nc = config["--n-conditions"];
    nd = config["--n-dep-vars"];
    ni = config["--n-items"];
    ng = nc * nd;
    nrows = ng * ni;

    nobs = config["--n-obs"];
    
    if (config["--n-obs"].is_array()){
        if (config["--n-obs"].size() != ng){
            throw std::invalid_argument( "Size of --n-obs does not match the size of the experiment.");
        }
        true_nobs = config["--n-obs"].get<std::vector<int>>();
    }else if (config["--n-obs"].is_number()){
        // Broadcase the given --n-obs to a vector of length `ng`
        true_nobs = std::vector<int>(ng, config["--n-obs"]);
    }
    if (config["--n-obs"].get<int>() == 0){
        isNRandomized = true;
        nobs = RNGEngine->genSampleSize(0.75, 20, 100, 300);
        std::fill(true_nobs.begin(), true_nobs.end(), nobs);
    }
    
    if (config["--means"].is_array()){
        if (config["--means"].size() != ng){
            throw std::invalid_argument( "Size of --means does not match the size of the experiment.");
        }
        true_means = config["--means"].get<std::vector<double>>();
    }else if (config["--means"].is_number()){
        // Broadcase the given --means to a vector of length `ng`
        true_means = std::vector<double>(ng, config["--means"]);
    }else{
        throw std::invalid_argument("--means is invalid or not provided.");
    }
    
    if (config["--sds"].is_array()){
        if (config["--sds"].size() != ng){
            throw std::invalid_argument( "Size of --sds does not match the size of the experiment.");
        }
        true_sds = config["--sds"].get<std::vector<double>>();
    }else if (config["--sds"].is_number()){
        // Broadcast the given --sds to a vector of length `ng`
        true_sds = std::vector<double>(ng, config["--sds"]);
    }else{
        throw std::invalid_argument("--sds is invalid or not provided.");
    }
    
    // I can drop this and only check if `cov == 0` and assume that the user wants
    // to run a correlated model
    // TODO: I need to change the if here
    if (config["--is-correlated"] || experimentType == ExperimentType::LatentModel) {
        isCorrelated = true;
        
        if (config["--covs"].is_array()){
            if (config["--covs"].size() != ng){
                throw std::invalid_argument( "Size of --covs does not match the size of the experiment.");
            }
            // DOC: Notify the user that the `sds` will be discarded.
            true_sigma = config["--covs"].get<std::vector<std::vector<double>>>();
            
        }else if (config["--covs"].is_number()){
            // Broadcase the --covs to the a matrix, and replace the diagonal values with
            // the value already given by --sds.
            double cov = config["--covs"];
            for (int g = 0; g < ng; g++) {
                true_sigma.push_back(std::vector<double>(ng, cov));
                true_sigma[g][g] = true_sds[g];
            }
        }else{
            throw std::invalid_argument("--covs is invalid or not provided.");
        }
    }

    // Factor Loading ...
    // CHECK: I think there are `ni` of these
    if (config["--loadings"].is_array()){
        if (config["--loadings"].size() != ni){
            throw std::invalid_argument( "Size of --loadings does not match the size of the experiment.");
        }
        factorLoadings = config["--loadings"].get<std::vector<double>>();
    }else if (config["--loadings"].is_number()){
        // Broadcast the given --loadings to a vector of length `ng`
        factorLoadings = std::vector<double>(ni, config["--loadings"]);
    }else{
        throw std::invalid_argument("--loadings is invalid or not provided.");
    }
    
    // Error's Standard Deviations
    if (config["--err-sds"].is_array()){
        if (config["--err-sds"].size() != nrows){
            throw std::invalid_argument( "Size of --err-sds does not match the size of the experiment.");
        }
        errorSD = config["--err-sds"].get<std::vector<double>>();
    }else if (config["--err-sds"].is_number()){
        // Broadcast the given --err-sds to a vector of length `nrows`
        errorSD = std::vector<double>(nrows, config["--err-sds"]);
    }else{
        throw std::invalid_argument("--err-sds is invalid or not provided.");
    }
    
    // Error's Covariant Matrix
    if (config["--err-covs"].is_array()){
        if (config["--err-covs"].size() != nrows){
            throw std::invalid_argument( "Size of --err-covs does not match the size of the experiment.");
        }
        errorCov = config["--err-covs"].get<std::vector<std::vector<double>>>();
        
    }else if (config["--err-covs"].is_number()){
        // Broadcase the --err-covs to the a matrix, and replace the diagonal values with
        // the value already given by --sds.
        double cov = config["--err-covs"];
        for (int r = 0; r < nrows; r++) {
            errorCov.push_back(std::vector<double>(nrows, cov));
            errorCov[r][r] = errorSD[r];
        }
    }else{
        throw std::invalid_argument("--err-covs is invalid or not provided.");
    }
    
}

void ExperimentSetup::randomize_nObs() {
    nobs = RNGEngine->genSampleSize(0.75, 20, 100, 300);
    std::fill(true_nobs.begin(), true_nobs.end(), nobs);
}

