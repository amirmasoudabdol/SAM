//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENTSETUP_H
#define SAMPP_EXPERIMENTSETUP_H

#include "sam.h"

#include "Utilities.h"

#include <variant>

namespace sam {

using json = nlohmann::json;

// Forward declration of the necessary classes.
class ExperimentSetupBuilder;

/// \brief An abstract class for a random variable parameter.
///
/// This is designed to capture the encapsulate a distribution and therefore
/// mimic the behavior or a random variable.
///
/// \todo Implement a copy constructor that can handle the copy
/// from arma::Row<T>
template <typename T>
class Parameter : public arma::Row<T> {
  
  std::variant<std::monostate, Distribution, MultivariateDistribution> dist;
  
public:
  Parameter() : arma::Row<T>() {};
    
  Parameter(const json &j, size_t size) {
    
    this->resize(size);
    
    arma::Col<T> val(size);
    
    std::cout << "Constructed!\n";
    switch (j.type()) {
      case nlohmann::detail::value_t::array: {
        val = arma::Col<T>(j.get<std::vector<T>>());
      } break;
        
      case nlohmann::detail::value_t::number_integer:
      case nlohmann::detail::value_t::number_unsigned:
      case nlohmann::detail::value_t::number_float:
        val = std::vector<T>(size, j.get<T>());
        break;
        
      case nlohmann::detail::value_t::object: {
        std::cout << j.dump(4) << std::endl;
        std::string name = j.at("dist").get<std::string>();
        if (name.find("mv") != std::string::npos) {
          /// Multivariante Distribution
          dist = make_multivariate_distribution(j);
          auto v = Random::get(std::get<2>(dist));
          val.imbue([&, i = 0]() mutable {
            return static_cast<T>(v[i++]);
          });
        }else{
          /// Univariate Distribution
          dist = make_distribution(j);
          auto v = static_cast<T>(Random::get(std::get<1>(dist)));
          val = arma::Col<T>(std::vector<T>(size, v));
        }
      } break;
        
      case nlohmann::detail::value_t::null:
      default:
        throw std::invalid_argument("Missing parameter.\n");
        break;
    }
    
    this->imbue([&, i = 0]() mutable {
      return val[i++];
    });
  }
  
  void randomize() {
    if (dist.index() != 0) {
      std::visit(overload {
        [&](Distribution &d) {
          auto v = static_cast<T>(Random::get(d));
          this->fill(v);
          return;
        },
        [&](MultivariateDistribution &md) {
          auto v = Random::get(md);
          this->imbue([&, i = 0]() mutable {
            return static_cast<T>(v[i++]);
          });
          return;
        },
        [&](auto &monostate) {
          return;
        }
      }, dist);
    }
  }
  
};

///
/// \brief      Define a class for ExperimentSetup.
///
/// ExperimentSetup contains the necessary parameters for initiating and
/// generating the data needed for the Experiment.
///
class ExperimentSetup {

  friend class ExperimentSetupBuilder;

  //! Number of experimental conditions, e.g., treatment 1, treatment 2.
  int nc_{0};

  //! Number of _dependent variables_ in each experimental condition.
  int nd_{0};

  //! Total number of groups. Always calculated as
  //! \f$n_g = n_c \times n_d\f$, unless the simulation contains latent
  //! variables, \f$n_g = n_c \times n_d \times n_i\f$
  int ng_{0};
  
  int n_reps_{1};

  ///! Indicates the number of observations in each group
  Parameter<int> nobs_;

public:

  ///
  /// Create a new ExperimentSetup by invoking a ExperimentSetupBuilder.
  ///
  /// \return     An instance of the builder.
  ///
  static ExperimentSetupBuilder create();

  /// Default constructor of the ExperimentSetup. This is necessary because of
  /// the ExperimentSetupBuilder
  ExperimentSetup() = default;

  explicit ExperimentSetup(json &config);

  //! Test Strategy Parameters
  json tsp_conf;

  //! Data Strategy Parameters
  json dsp_conf;

  //! Effect Estimator Parameters
  json esp_conf;

  const int nc() const { return nc_; };
  const int nd() const { return nd_; };
  const int ng() const { return ng_; };
  const int nreps() const { return n_reps_; };

  const arma::Row<int> &nobs() const { return nobs_; };
//  void set_nobs(arma::Row<int> &val) { nobs_ = val; };

  void randomizeTheParameters();
};

class ExperimentSetupBuilder {

  //! Experiment Setup instance that it's going to be build
  ExperimentSetup setup;

  //! Used to make sure that experiment setup has the correct size
  bool is_expr_size_decided{false};

  int seed{-1};

  /// Calculate the experiment setup sizes
  void calculate_experiment_size() {
    setup.ng_ = setup.nc_ * setup.nd_;

    if (setup.ng_ == 0) {
      is_expr_size_decided = false;
    } else {
      is_expr_size_decided = true;
    }
  }

  /// Check if variable sizes are set properly, if not, tries to calcualte
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

  /// \brief Create and configure a new experiment setup based on the given
  /// configuration.
  ExperimentSetupBuilder &fromConfigFile(json &config);

  /// \brief      Sets the seed for randomizing setup parameters
  ///
  /// \param[in]  s     seed
  ///
  /// \return     A reference to the builder
  ExperimentSetupBuilder &setSeed(const int s) {
    seed = s;
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

    fill_vector<int>(setup.nobs_, setup.ng_, nobs);

    return *this;
  }

  ExperimentSetupBuilder &setNumObservations(const arma::Row<int> &nobs) {
    check_expr_size();

    if (nobs.size() != setup.ng_) {
      throw std::length_error(
          "The size of nobs does not match size of the experiment.");
    }

    // If everything ok, replace the vector with the given vector
    /// \todo: Recover this, it's been removed during the Parameter transition
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

    if (seed == -1) {
      seed = rand();
    }

    return setup;
  }
};

} // namespace sam

#endif // SAMPP_EXPERIMENTSETUP_H
