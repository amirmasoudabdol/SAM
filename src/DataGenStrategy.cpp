//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <DataGenStrategy.h>
#include <iostream>
#include "gsl/gsl_statistics.h"

#include "Utilities.h"

std::vector<std::vector<double>> FixedEffectStrategy::genData()  {
    // TODO: This can actually call `genNewObservationForAllGroups`
    if (!setup.isMultivariate){
        return this->rngEngine.normal(setup.true_means, setup.true_vars, setup.nobs);
    }else{
        return this->rngEngine.mvnorm(setup.true_means, setup.true_sigma, setup.nobs);
    }
}

std::vector<std::vector<double>> FixedEffectStrategy::genNewObservationsForAllGroups(int n_new_obs) {
    if (!setup.isMultivariate){
        return this->rngEngine.normal(setup.true_means, setup.true_vars, n_new_obs);
    }else{
        return this->rngEngine.mvnorm(setup.true_means, setup.true_sigma, n_new_obs);
    }
}

std::vector<double> FixedEffectStrategy::genNewObservationsFor(int g, int n_new_obs) {

    return std::vector<double>();
}


// LatentDataStrategy

// This is different from other genData, since it should generate items, but will
// only return the means!
std::vector<std::vector<double>> LatentDataStrategy::genData()  {

//    std::cout << "LatentModel, GenData Started!\n";

    std::vector<std::vector<double> > items;
    std::vector<double> means;
    
    // Measurement here is technically the mean of each row, it'll be
    // ng * nobs
    std::vector<std::vector<double> > meas;
    
    
    int ni = setup.ni;
    int nc = setup.nc;
    int nd = setup.nd;
    int nrows = setup.nrows;       // Total number of rows
    
    int nobs = setup.nobs;
    
    // This is correct, you have one a for each item.
    gsl_vector* lambda = gsl_vector_alloc(ni);
    lambda->data = setup.factorLoadings.data();
    // gsl_vector_set_all(lambda, 0.1);
    
    // DV ---------------------------------------------
    // Mean of each dv
    gsl_vector* dvMeans = gsl_vector_calloc(nd);    // this needs to be broadcasted to ng * ng
    // double dvm[] = {0, 5, 10};
    dvMeans->data = setup.factorMeans.data();
    
    // gsl_vector* dvVars = gsl_vector_calloc(nd);
    // gsl_vector_set_all(dvVars, 1);
    
    gsl_matrix* dvSigma = gsl_matrix_alloc(nd, nd);     // this needs to be broadcasted to ng * ng
    gsl_matrix_set_identity(dvSigma);                   // Set to 1 diagonally
    dvSigma->data = flatten(setup.factorCov).data();
    // ------------------------------------------------
    
//    gsl_vector* errorMeans = gsl_vector_calloc(ni);     // Usually 0.
    
    // gsl_matrix* errorSigma = gsl_matrix_calloc(ni, ni);
    // gsl_matrix_set_identity(errorSigma);
    // gsl_matrix_scale(errorSigma, 0.9);
    // errorSigma->data = flatten(setup.errorCov).data();
    
    
    gsl_rng_env_setup();

    const gsl_rng_type *T;
    T = gsl_rng_default;
    
    gsl_rng* rng_stream;
    rng_stream = gsl_rng_alloc(T);
    
    gsl_matrix* factorScores = gsl_matrix_calloc(nd, nobs);
//    gsl_matrix* itemErrors = gsl_matrix_calloc(ni, nobs);
    
    gsl_vector* scoreCol = gsl_vector_calloc(nd);
//    gsl_vector* errorCol = gsl_vector_calloc(ni);
    
    for (int n = 0; n < nobs; n++) {
        gsl_ran_multivariate_gaussian(rng_stream, dvMeans, dvSigma, scoreCol);
        gsl_matrix_set_col(factorScores, n, scoreCol);
        
//        gsl_ran_multivariate_gaussian(rng_stream, errorMeans, errorSigma, errorCol);
//        gsl_matrix_set_col(itermErrors, n, errorCol);
    }
    
    gsl_vector* allErrorMeans = gsl_vector_calloc(nrows);
    
    gsl_matrix* allErrorsSigma = gsl_matrix_calloc(nrows, nrows);
    // gsl_matrix_set_identity(allErrorsSigma);
    // gsl_matrix_scale(allErrorsSigma, 0.9);
    allErrorsSigma->data = flatten(setup.errorCov).data();
    
    gsl_matrix* allErrors = gsl_matrix_calloc(nrows, nobs);
    
    gsl_vector* allErrorCol = gsl_vector_calloc(nrows);
    for (int n = 0; n < nobs; n++) {
        gsl_ran_multivariate_gaussian(rng_stream, allErrorMeans, allErrorsSigma, allErrorCol);
        gsl_matrix_set_col(allErrors, n, allErrorCol);
    }
    
    gsl_matrix* allScores = gsl_matrix_calloc(nrows, nobs);
//    gsl_vector* tempRow = gsl_vector_calloc(ni);
    
    int row = 0;
    int col = 0;
    
    for (int c = 0; c < nc; c++) {

        for (int d = 0 ; d < nd; d++) {
            row = c * nd + d;
            for (int n = 0; n < nobs; n++){
                col = n;
                for (int i = 0; i < ni; i++) {
                    gsl_matrix_set(allScores, row * ni + i, col,
                                       gsl_vector_get(dvMeans, d) +
                                       gsl_matrix_get(factorScores, d, col) * gsl_vector_get(lambda, i) +
                                       gsl_matrix_get(allErrors, row * ni + i, col)
                                   );
                    
//                    std::cout << row * ni + i << ", " << col << ": " << gsl_matrix_get(allScores, row * ni + i, col)
//                                    << " - " << gsl_vector_get(dvMeans, d)
//                                    << std::endl;
                    
                    // I can compute mean of each row (of items) here and put it into `measurements` if necessary
                }
            }
        }
    }
    
    gsl_vector* itemMeans = gsl_vector_calloc(nrows);
    gsl_vector* tmpRow = gsl_vector_alloc(nobs);
    for (int n = 0; n < nrows; n++) {
        gsl_matrix_get_row(tmpRow, allScores, n);
        gsl_vector_set(itemMeans, n, gsl_stats_mean(tmpRow->data, 1, nobs));
//        std::cout << gsl_vector_get(itemMeans, n) << std::endl;
    }
    
    for (int r = 0; r < nrows; r++) {
        items.push_back(std::vector<double>(nobs));
        for (int n = 0; n < nobs; n++) {
            items[r][n] = gsl_matrix_get(allScores, r, n);
        }
        means.push_back(gsl_vector_get(itemMeans, r));
    }
    
    int rowOffset = 0;
    gsl_vector* v = gsl_vector_alloc(nrows);
//    gsl_matrix_view dv_items;

    
    for (int r = 0; r < nrows; r++) {
        meas.push_back(std::vector<double>(nobs));
    }
        
    for (int n = 0; n < nobs; n++) {
        gsl_matrix_get_col(v, allScores, n);
            
        for (int c = 0; c < nc; c++) {
                
            for (int d = 0; d < nd; d++) {
                rowOffset = c * nd + d;
            
                gsl_vector_view vsub = gsl_vector_subvector(v, rowOffset * ni, ni);
//                std::cout << gsl_stats_mean(vsub.vector.data, 1, ni) << "\n";
                
                meas[c * nd + d][n] = gsl_stats_mean(vsub.vector.data, 1, ni); // << "\n";
                
//                 std::cout << c * nd + d << ", " << meas[c * nd + d][n] << ", ";
//                std::cout << c * nd + d << ", " << n << "\n";
//                                rowOffset * ni << ", " << rowOffset * ni + ni << "\n";
            
            
            }
//            std::cout << "\n";
        }
        
    }
    
//    std::cout << meas.size() << ": size\n" << meas[0].size() << " :<--\n";

    // REMEMEBR:
    // FIXME: This is the incorrent measurements that I'm returning. I need to be careful here.
    return meas;
    // return this->rngEngine.normal(setup.true_means, setup.true_vars, setup.nobs);
}

std::vector<std::vector<double>> LatentDataStrategy::genNewObservationsForAllGroups(int n_new_obs) {
    return this->rngEngine.normal(setup.true_means, setup.true_vars, n_new_obs);
}

std::vector<double> LatentDataStrategy::genNewObservationsFor(int g, int n_new_obs) {
    
    return std::vector<double>();
}

void LatentDataStrategy::latentModelTest() {
    
    std::vector<std::vector<double> > items;
    std::vector<double> means;
    std::vector<std::vector<double> > meas;
    
    
    int ni = 4;
    int nc = 2;
    int nd = 3;
    int ng = ni * nc * nd;       // Total number of rows
    
    int nobs = 50;
    
    // This is correct, you have one a for each item.
    gsl_vector* lambda = gsl_vector_alloc(ni);
    gsl_vector_set_all(lambda, 0.1);
    
    // DV ---------------------------------------------
    // Mean of each dv
    gsl_vector* dvMeans = gsl_vector_calloc(nd);    // this needs to be broadcasted to ng * ng
    double dvm[] = {0, 5, 10};
    dvMeans->data = dvm;
    
    gsl_vector* dvVars = gsl_vector_calloc(nd);
    gsl_vector_set_all(dvVars, 1);
    
    gsl_matrix* dvSigma = gsl_matrix_alloc(nd, nd);     // this needs to be broadcasted to ng * ng
    gsl_matrix_set_identity(dvSigma);                   // Set to 1 diagonally
    // ------------------------------------------------
    
//    gsl_vector* errorMeans = gsl_vector_calloc(ni);     // Usually 0.
    
    gsl_matrix* errorSigma = gsl_matrix_calloc(ni, ni);
    gsl_matrix_set_identity(errorSigma);
    gsl_matrix_scale(errorSigma, 0.9);
    
    
    gsl_rng_env_setup();

    const gsl_rng_type *T;
    T = gsl_rng_default;
    
    gsl_rng* rng_stream;
    rng_stream = gsl_rng_alloc(T);
    
    gsl_matrix* factorScores = gsl_matrix_calloc(nd, nobs);
//    gsl_matrix* itemErrors = gsl_matrix_calloc(ni, nobs);
    
    gsl_vector* scoreCol = gsl_vector_calloc(nd);
//    gsl_vector* errorCol = gsl_vector_calloc(ni);
    
    for (int n = 0; n < nobs; n++) {
        gsl_ran_multivariate_gaussian(rng_stream, dvMeans, dvSigma, scoreCol);
        gsl_matrix_set_col(factorScores, n, scoreCol);
        
//        gsl_ran_multivariate_gaussian(rng_stream, errorMeans, errorSigma, errorCol);
//        gsl_matrix_set_col(itermErrors, n, errorCol);
    }
    
    gsl_vector* allErrorMeans = gsl_vector_calloc(ng);
    
    gsl_matrix* allErrorsSigma = gsl_matrix_calloc(ng, ng);
    gsl_matrix_set_identity(allErrorsSigma);
    gsl_matrix_scale(allErrorsSigma, 0.9);
    
    gsl_matrix* allErrors = gsl_matrix_calloc(ng, nobs);
    
    gsl_vector* allErrorCol = gsl_vector_calloc(ng);
    for (int n = 0; n < nobs; n++) {
        gsl_ran_multivariate_gaussian(rng_stream, allErrorMeans, allErrorsSigma, allErrorCol);
        gsl_matrix_set_col(allErrors, n, allErrorCol);
    }
    
    gsl_matrix* allScores = gsl_matrix_calloc(ng, nobs);
//    gsl_vector* tempRow = gsl_vector_calloc(ni);
    
    int row = 0;
    int col = 0;
    
    for (int c = 0; c < nc; c++) {

        for (int d = 0 ; d < nd; d++) {
            row = c * nd + d;
            for (int n = 0; n < nobs; n++){
                col = n;
                for (int i = 0; i < ni; i++) {
                    gsl_matrix_set(allScores, row * ni + i, col,
                                       gsl_vector_get(dvMeans, d) +
                                       gsl_matrix_get(factorScores, d, col) * gsl_vector_get(lambda, i) +
                                       gsl_matrix_get(allErrors, row * ni + i, col)
                                   );
                    
//                    std::cout << row * ni + i << ", " << col << ": " << gsl_matrix_get(allScores, row * ni + i, col)
//                                    << " - " << gsl_vector_get(dvMeans, d)
//                                    << std::endl;
                    
                    // I can compute mean of each row (of items) here and put it into `measurements` if necessary
                }
            }
        }
    }
    
    gsl_vector* itemMeans = gsl_vector_calloc(ng);
    gsl_vector* tmpRow = gsl_vector_alloc(nobs);
    for (int n = 0; n < ng; n++) {
        gsl_matrix_get_row(tmpRow, allScores, n);
        gsl_vector_set(itemMeans, n, gsl_stats_mean(tmpRow->data, 1, nobs));
//        std::cout << gsl_vector_get(itemMeans, n) << std::endl;
    }
    
    for (int g = 0; g < ng; g++) {
        items.push_back(std::vector<double>(nobs));
        for (int n = 0; n < nobs; n++) {
            items[g][n] = gsl_matrix_get(allScores, g, n);
        }
        means.push_back(gsl_vector_get(itemMeans, g));
    }
    
    int rowOffset = 0;
    gsl_vector* v = gsl_vector_alloc(ng);
//    gsl_matrix_view dv_items;

    
    for (int g = 0; g < ng; g++) {
        meas.push_back(std::vector<double>(nobs));
    }
        
    for (int n = 0; n < nobs; n++) {
        gsl_matrix_get_col(v, allScores, n);
            
        for (int c = 0; c < nc; c++) {
                
            for (int d = 0; d < nd; d++) {
                rowOffset = c * nd + d;
            
                gsl_vector_view vsub = gsl_vector_subvector(v, rowOffset * ni, ni);
//                std::cout << gsl_stats_mean(vsub.vector.data, 1, ni) << "\n";
                
                meas[c * nd + d][n] = gsl_stats_mean(vsub.vector.data, 1, ni); // << "\n";
                
//                std::cout << c * nd + d << ", " << n << "\n";
//                                rowOffset * ni << ", " << rowOffset * ni + ni << "\n";
            
            
            }
        }
    }
    
    

    gsl_vector_free(lambda);
    gsl_vector_free(dvMeans);
    gsl_vector_free(dvVars);
//    gsl_vector_free(errorMeans);
    gsl_vector_free(scoreCol);
//    gsl_vector_free(errorCol);
    gsl_vector_free(allErrorMeans);
    gsl_vector_free(allErrorCol);
//    gsl_vector_free(tempRow);
    gsl_vector_free(itemMeans);
    gsl_vector_free(tmpRow);
    gsl_vector_free(v);

    gsl_matrix_free(dvSigma);
    gsl_matrix_free(errorSigma);
    gsl_matrix_free(factorScores);
//    gsl_matrix_free(itemErrors);
    gsl_matrix_free(allErrorsSigma);
    gsl_matrix_free(allErrors);
    gsl_matrix_free(allScores);
    
    
//    std::cout << "Done!\n";
    
    
    
}
