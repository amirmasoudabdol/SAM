//===-- ExperimentSetup.h - Experiment Setup Module -----------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-22.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the declaration of Experiment Setup module, which
/// stores the information about the experiment design.
///
//===----------------------------------------------------------------------===//

#ifndef SAMPP_EXPERIMENTSETUP_H
#define SAMPP_EXPERIMENTSETUP_H

#include "sam.h"

#include "Utilities.h"
#include "Parameter.h"

#include <variant>

namespace sam {

class ExperimentSetupBuilder;

/// @brief      Declaraton of ExperimentSetup class
///
/// ExperimentSetup stores all the necessary parameters concerning the experiment
/// design, e.g., number of conditions, number of dependent variables, etc. It
/// also stores a copy of the data, test, and effect strategies' configurations.
///
/// @ingroup    Experiment
class ExperimentSetup {

  friend class ExperimentSetupBuilder;

  //! Number of experimental conditions, e.g., treatment 1, treatment 2.
  int nc_{0};

  //! Number of _dependent variables_ in each experimental condition.
  int nd_{0};

  //! Total number of groups. Always calculated as
  int ng_{0};
  
  //! Total number of replications
  int n_reps_{1};

  ///! Indicates the number of observations in each group
  Parameter<int> nobs_;

public:

  /// Create a new ExperimentSetup by invoking a ExperimentSetupBuilder.
  ///
  /// @return     An instance of the builder.
  static ExperimentSetupBuilder create();

  ExperimentSetup() = default;

  explicit ExperimentSetup(json &config);

  //! Test Strategy Parameters
  json tsp_conf;

  //! Data Strategy Parameters
  json dsp_conf;

  //! Effect Estimator Parameters
  json esp_conf;

  /// Returns the number of conditions
  [[nodiscard]] int nc() const { return nc_; };
  
  /// Returns the number of dependent variables in each conditions
  [[nodiscard]] int nd() const { return nd_; };
  
  /// Returns the total number of groups
  ///
  /// This is an internal parameters mainly used to iterate over conditions and
  /// dependent variables, and it's calculated as \f$n_g = n_c \times n_d\f$
  [[nodiscard]] int ng() const { return ng_; };
  
  /// Returns the total number of _planned_ replications
  [[nodiscard]] int nreps() const { return n_reps_; };

  /// Returns the _original_ number of observations per group
  const arma::Row<int> &nobs() const { return nobs_; };
//  void set_nobs(arma::Row<int> &val) { nobs_ = val; };

  void randomizeTheParameters();
};

/// @ingroup AbstractBuilders
class ExperimentSetupBuilder {

  //! Experiment Setup instance that it's going to be build
  ExperimentSetup setup;

  //! Used to make sure that experiment setup has the correct size
  bool is_expr_size_decided{false};

  /// Calculate the experiment setup sizes
  void calculate_experiment_size() {
    setup.ng_ = setup.nc_ * setup.nd_;

    if (setup.ng_ == 0) {
      is_expr_size_decided = false;
    } else {
      is_expr_size_decided = true;
    }
  }

  /// Check if variable sizes are set properly, if not, tries to calculate
  /// them, if fails, it'll throw and error.
  void check_expr_size() {
    if (!is_expr_size_decided) {
      calculate_experiment_size();
      if (!is_expr_size_decided) {
        throw std::invalid_argument(
            "Number of groups cannot be 0. Make sure that \
                            you have set the number of conditions or dependent variables first, with non-zero \
                            values.");
      }
    }
  }

public:
  ExperimentSetupBuilder() = default;

  /// @brief Create and configure a new experiment setup based on the given
  /// configuration.
  ExperimentSetupBuilder &fromConfigFile(json &config);

  /// @brief      Sets the seed for randomizing setup parameters
  ///
  /// @param[in]  s     seed
  ///
  /// @return     A reference to the builder
  ExperimentSetupBuilder &setSeed(const int s) {
    return *this;
  }

  ExperimentSetupBuilder &setNumConditions(const int nc) {
    if (nc <= 0) {
      throw std::invalid_argument("Number of conditions cannot be 0.");
    }
    setup.nc_ = nc;
    calculate_experiment_size();

    return *this;
  }

  ExperimentSetupBuilder &setNumDependentVariables(const int nd) {
    if (nd <= 0) {
      throw std::invalid_argument("Number of dependent variables cannot be 0.");
    }
    setup.nd_ = nd;
    calculate_experiment_size();

    return *this;
  }

  ExperimentSetupBuilder &setNumItems(const int ni) { return *this; }

  ExperimentSetupBuilder &setNumObservations(const int nobs) {
    check_expr_size();

//    fill_vector<int>(setup.nobs_, setup.ng_, nobs);

    return *this;
  }

  ExperimentSetupBuilder &setNumObservations(const arma::Row<int> &nobs) {
    check_expr_size();

    if (nobs.size() != setup.ng_) {
      throw std::length_error(
          "The size of nobs does not match size of the experiment.");
    }

    // If everything ok, replace the vector with the given vector
    /// @todo: Recover this, it's been removed during the Parameter transition
//    setup.nobs_ = nobs;

    return *this;
  }

  ExperimentSetupBuilder &
  setTestStrategyParameters(json &test_strategy_config) {
    setup.tsp_conf = test_strategy_config;
    return *this;
  }

  ExperimentSetupBuilder &
  setDataStrategyParameters(json &data_strategy_config) {
    setup.dsp_conf = data_strategy_config;
    return *this;
  }

  ExperimentSetupBuilder &
  setEffectStrategyParameters(json &effect_strategy_config) {
    setup.esp_conf = effect_strategy_config;
    return *this;
  }

  ExperimentSetup build() {
    check_expr_size();

    return setup;
  }
};

} // namespace sam

#endif // SAMPP_EXPERIMENTSETUP_H
