//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <stdexcept>

#include "ExperimentSetup.h"

std::ostream& operator<<(std::ostream& os, ExperimentType et)
{
    switch(et)
    {
        case FixedModel   : os << "Fixed Model";    break;
        case RandomModel      : os << "Random Model"; break;
        case LatentModel     : os << "Latent Model";  break;
        default    : os.setstate(std::ios_base::failbit);
    }
    return os;
}

//ExperimentSetup::ExperimentSetup(int n_conditions, int n_dvs, int n_obs, std::vector<double> means, std::vector<double> vars)
//        : nc(n_conditions), nd(n_dvs), nobs(n_obs), true_means(means), true_vars(vars) {
//    ng = nc * nd;
//}


ExperimentSetup::ExperimentSetup(json& config) {
    
    if (config["--data-strategy"] == "FixedModel"){
         experimentType = FixedModel;
        
        nc = config["--n-conditions"];
        nd = config["--n-dep-vars"];
        ng = nc * nd;

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
        
        if (config["--means"].is_array()){
            if (config["--means"].size() != ng){
                throw std::invalid_argument( "Size of --means does not match the size of the experiment.");
            }
            true_means = config["--means"].get<std::vector<double>>();
        }else if (config["--means"].is_number()){
            // Broadcase the given --means to a vector of length `ng`
            true_means = std::vector<double>(ng, config["--means"]);
        }
        
        if (config["--sds"].is_array()){
            if (config["--sds"].size() != ng){
                throw std::invalid_argument( "Size of --sds does not match the size of the experiment.");
            }
            true_sds = config["--sds"].get<std::vector<double>>();
        }else if (config["--sds"].is_number()){
            // Broadcast the given --sds to a vector of length `ng`
            true_sds = std::vector<double>(ng, config["--sds"]);
        }
        
        // I can drop this and only check if `cov == 0` and assume that the user wants
        // to run a correlated model
        if (config["--is-correlated"]){
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
            }
        }
    
    }

    if (config["--data-strategy"] == "LatentModel") {
        experimentType = LatentModel;
        
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
        
        if (config["--means"].is_array()){
            if (config["--means"].size() != ng){
                throw std::invalid_argument( "Size of --means does not match the size of the experiment.");
            }
            true_means = config["--means"].get<std::vector<double>>();
        }else if (config["--means"].is_number()){
            // Broadcase the given --means to a vector of length `ng`
            true_means = std::vector<double>(ng, config["--means"]);
        }
        
        if (config["--sds"].is_array()){
            if (config["--sds"].size() != ng){
                throw std::invalid_argument( "Size of --sds does not match the size of the experiment.");
            }
            true_sds = config["--sds"].get<std::vector<double>>();
        }else if (config["--sds"].is_number()){
            // Broadcast the given --sds to a vector of length `ng`
            true_sds = std::vector<double>(ng, config["--sds"]);
        }
        
        
        // We assume that it's a correlated study anyway, I need the RandomNumberGenerator to
        // be able to generate multivariate
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
        }
        
    }
    
}
