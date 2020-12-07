//
// Created by Amir Masoud Abdol on 2019-01-22.
//

///
/// @defgroup   DataStrategies Data Strategies
/// @brief      List of available Data Strategies
///
/// Description to come!
///

#ifndef SAMPP_DATAGENSTRATEGY_H
#define SAMPP_DATAGENSTRATEGY_H

#include "sam.h"

#include "ExperimentSetup.h"

using Random = effolkronium::random_static;

namespace sam {

class Experiment;
class ExperimentSetup;

///
/// @brief      Abstract class for Data Strategies
///
/// A DataGenStrategy should at least two methods, `genData` and
/// `genNewObservationForAllGroups`. The former is mainly used to populate a new
/// Experiment while the latter is being used by some hacking strategies, e.g.
/// OptionalStopping, where new data — from the same population — is needed.
///
class DataStrategy {

public:
  enum class DataModel { LinearModel, LatentModel, GradedResponseModel };

  /// Factory method for DataStrategy.
  /// 
  /// @param setup An instance of ExperimentSetup
  /// @return a new DataStrategy
  static std::unique_ptr<DataStrategy> build(json &data_strategy_config);

  /// Pure deconstructor of the DataStrategy abstract class.
  virtual ~DataStrategy() = 0;

  /// Read a CSV file and load the data into the measurement. Each column
  /// is considered to be one group, based on the information already
  /// provided in the `experiment.setup`.
  /// 
  /// @param expr A pointer to the experiment
  /// @param filename The CSV filename
  void loadRawData(Experiment *expr, const std::string &filename);

  /// Populates the `experiment->measurements` with data based on the parameters
  /// specified in `setup`.
  /// 
  /// @param experiment A pointer to an Experiment object
  virtual void genData(Experiment *experiment) = 0;

  /// Generates `n_new_obs` new observations to each group.
  /// 
  /// @note This routine uses the secondary random number stream to avoid
  /// conflicting with the main random engine.
  /// 
  /// @param experiment The pointer to the current experiment
  /// @param n_new_obs The number of new observations to be added
  /// 
  /// @return An array of new observations

  virtual std::vector<arma::Row<double>>
  genNewObservationsForAllGroups(Experiment *experiment, int n_new_obs) = 0;

  ///
  /// Generate `n_new_obs` new observations for `g` group.

  /// @param experiment The pointer to the experiment
  /// @param g The target group
  /// @param n_new_obs The number of new observations
  /// 
  /// @return An array of new observations
  virtual arma::Row<double> genNewObservationsFor(Experiment *experiment, int g,
                                                  int n_new_obs) = 0;
};

/// @brief Linear Model Data Strategy
///
/// @ingroup  DataStrategies
///
class LinearModelStrategy final : public DataStrategy {

public:
  
  /// @brief Parameters of LinearModelStrategy
  ///
  struct Parameters {
    DataModel name{DataModel::LinearModel};

    ///@{
    /// Distributions of main effects
    std::optional<std::vector<Distribution>> meas_dists;
    std::optional<MultivariateDistribution> m_meas_dist;
    ///@}

    ///@{
    /// Distributions of error.
    std::optional<std::vector<Distribution>> erro_dists;
    std::optional<MultivariateDistribution> m_erro_dist;
    ///@}

    Parameters() = default;
  };

  LinearModelStrategy(){};

  LinearModelStrategy(const Parameters p)
      : params(p){

        };

  virtual void genData(Experiment *experiment) override;

  virtual std::vector<arma::Row<double>>
  genNewObservationsForAllGroups(Experiment *experiment,
                                 int n_new_obs) override;

  virtual arma::Row<double> genNewObservationsFor(Experiment *experiment, int g,
                                                  int n_new_obs) override;

private:
  Parameters params;
};

// JSON Parser for LinearModelStrategy::Parameters
inline void to_json(json &j, const LinearModelStrategy::Parameters &p) {
  j = json{
      {"name", p.name}
      //                    {"measurements", p.m_meas_dist}
  };
  /// Issue: For now, I cannot actually write that back to the file. That's
  /// because `mdist` is actually a MultivariateDistribution type, and not a
  /// mvnorm, etc., that I already have a serializer for. I think I need to have
  /// another serializer, see #227.
}

inline void from_json(const json &j, LinearModelStrategy::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  if (j.at("measurements").type() == nlohmann::detail::value_t::object) {
    p.m_meas_dist = make_multivariate_distribution(j.at("measurements"));
  }

  if (j.at("measurements").type() == nlohmann::detail::value_t::array) {
    std::vector<Distribution> dists;
    for (const auto &value : j["measurements"]) {
      dists.push_back(make_distribution(value));
    }
    p.meas_dists = dists;
  }
  
  if (j.contains("errors")) {
    if (j.at("errors").type() == nlohmann::detail::value_t::object) {
      p.m_erro_dist = make_multivariate_distribution(j.at("errors"));
    }

    if (j.at("errors").type() == nlohmann::detail::value_t::array) {
      std::vector<Distribution> dists;
      for (const auto &value : j["errors"]) {
        dists.push_back(make_distribution(value));
      }
      p.erro_dists = dists;
    }
  }
}

//=================================================================================//

///
/// @brief ⚠️ TO BE IMPLEMENTED!
///
/// A Data Strategy for constructing a general [Structural Equaiton
/// Model](https://en.wikipedia.org/wiki/Structural_equation_modeling).
///
/// @ingroup  DataStrategies
///
/// @warning    To be implemented!
///
/// @note       LatentDataStrategy will generate individual items, therefore it
///             might be slower than other models.
///
class LatentDataStrategy final : public DataStrategy {

public:
  LatentDataStrategy() {}

  virtual void genData(Experiment *experiment) override;

  virtual std::vector<arma::Row<double>>
  genNewObservationsForAllGroups(Experiment *experiment,
                                 int n_new_obs) override;

  virtual arma::Row<double> genNewObservationsFor(Experiment *experiment, int g,
                                                  int n_new_obs) override;

private:
  //! A function for computing the latent variable
  //! This is rather new and I still need to implement it. It's going to
  //! use std::function, it's basically more efficient and less verbose
  //! Strategy Pattern ;P
  void latent_function();
};

///
/// @brief      Simulate data based on General Graded Response Model.
///
///
/// @ingroup  DataStrategies
///
///
/// @note:
/// - DVs in GRM are distinguished by their participant abilities to answer tests.
///   Therefore, we'll have `ng_` number of `abilities`. This value is being used
///   to, in each group, to initialize a normal distribution of `\theta ~
///   N(abilitis[i], 1)`.
/// - \f$ \beta \f$
///
class GRMDataStrategy final : public DataStrategy {
public:
  struct Parameters {

    DataModel name{DataModel::GradedResponseModel};

    int n_dims;

    //! Number of items
    int n_items;
    
    //! Number of categories
    int n_categories;
    
    //! Inidicates the Response Function
    std::string response_function {"Rasch"};

    std::optional<MultivariateDistribution> m_diff_dist;
    std::optional<MultivariateDistribution> m_abil_dist;

    std::optional<std::vector<Distribution>> diff_dists;
    std::optional<std::vector<Distribution>> abil_dists;

    Parameters() = default;
  };

  GRMDataStrategy(){};

  GRMDataStrategy(const Parameters &p);

  virtual void genData(Experiment *experiment) override;

  virtual std::vector<arma::Row<double>>
  genNewObservationsForAllGroups(Experiment *experiment,
                                 int n_new_obs) override;

  virtual arma::Row<double> genNewObservationsFor(Experiment *experiment, int g,
                                                  int n_new_obs) override;

private:
  Parameters params;

  /// Improvement: This can be replaced by `Random::get<bool>` but I need to
  /// test it first.
  Distribution uniform_dist = std::uniform_real_distribution<>{};

  arma::mat poa;        //! probablity of answering
  arma::umat responses; //! responses to items, binary
  arma::vec scores;     //! scores of each item

  arma::mat urand;

  //! Item difficulties
  arma::mat betas;

  //! Participants abilities
  arma::mat thetas;
  
  ///
  /// @brief      [Rasch Response Function](https://en.wikipedia.org/wiki/Rasch_model)
  ///
  /// @param[in]  theta  Indicates the participant's ability, θ
  ///
  /// @return     Sum score of `j`th participart over all items
  ///
  double rasch_score(const double theta);
};

// JSON Parser for GRMDataStrategy::Parameters
inline void to_json(json &j, const GRMDataStrategy::Parameters &p) {
  j = json{
      {"name", p.name},
      {"n_items", p.n_items},
      {"n_categories", p.n_categories},
      {"response_function", p.response_function},
      //                    {"difficulties", p.diff_dists},
      //                    {"abilities", p.abil_dists}
  };
  // Issue: There is similar to LinearModel where I cannot serialize a
  // Distribution to JSON yet.
}

inline void from_json(const json &j, GRMDataStrategy::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  j.at("n_items").get_to(p.n_items);
  j.at("n_categories").get_to(p.n_categories);
  j.at("response_function").get_to(p.response_function);

  // Collecting difficulties
  if (j.at("difficulties").type() == nlohmann::detail::value_t::object) {
    p.m_diff_dist = make_multivariate_distribution(j.at("difficulties"));
  } else {
    if (j.at("difficulties").type() == nlohmann::detail::value_t::array) {
      std::vector<Distribution> dists;
      for (const auto &value : j["difficulties"])
        dists.push_back(make_distribution(value));
      p.diff_dists = dists;
    }
  }

  // Collecting abilities
  if (j.at("abilities").type() == nlohmann::detail::value_t::object)
    p.m_abil_dist = make_multivariate_distribution(j.at("abilities"));
  else {
    if (j.at("abilities").type() == nlohmann::detail::value_t::array) {
      std::vector<Distribution> dists;
      for (const auto &value : j["abilities"])
        dists.push_back(make_distribution(value));
      p.abil_dists = dists;
    }
  }
}

NLOHMANN_JSON_SERIALIZE_ENUM(
    DataStrategy::DataModel,
    {{DataStrategy::DataModel::LinearModel, "LinearModel"},
     {DataStrategy::DataModel::LatentModel, "LatentModel"},
     {DataStrategy::DataModel::GradedResponseModel, "GradedResponseModel"}})

} // namespace sam

#endif // SAMPP_DATAGENSTRATEGY_H
