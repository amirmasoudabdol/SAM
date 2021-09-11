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
/// @ingroup DataStrategies
class DataStrategy {

public:
  enum class DataModel { LinearModel, LatentModel, GradedResponseModel };

  /// @brief      DataStrategy Factory Method
  ///
  /// @param      data_strategy_config  The data strategy configuration
  ///
  /// @return     A unique_ptr to a new DataStrategy
  static std::unique_ptr<DataStrategy> build(json &data_strategy_config);

  /// @brief Pure destructors of the DataStrategy abstract class.
  virtual ~DataStrategy() = 0;

  ///
  /// @brief Generates data based on the selected DataModel
  ///
  /// Populates the `experiment->groups_->measurements` with data based on the parameters
  /// specified in `setup`.
  ///
  /// @param experiment A pointer to an Experiment object
  virtual void genData(Experiment *experiment) = 0;

  /// @brief Generates `n_new_obs` new observations for each group
  ///
  /// @param experiment The pointer to the current experiment
  /// @param n_new_obs The number of new observations to be generated
  ///
  /// @return An array of new observations
  virtual std::vector<arma::Row<float>>
  genNewObservationsForAllGroups(Experiment *experiment, int n_new_obs) = 0;

  ///
  /// @brief Generate `n_new_obs` new observations for `g` group.
  ///
  /// @param experiment The pointer to the experiment
  /// @param g The target group
  /// @param n_new_obs The number of new observations
  ///
  /// @return An array of new observations for group `g`
//  virtual arma::Row<float> genNewObservationsFor(Experiment *experiment, int g,
//                                                  int n_new_obs) = 0;
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
    std::optional<std::vector<UnivariateDistribution>> meas_dists;
    std::optional<MultivariateDistribution> m_meas_dist;
    ///@}

    ///@{
    /// Distributions of error.
    std::optional<std::vector<UnivariateDistribution>> erro_dists;
    std::optional<MultivariateDistribution> m_erro_dist;
    ///@}
    
    arma::Row<float> means;
    arma::Mat<float> sigma;
    
    float tau2 {0};
    
    std::optional<std::vector<UnivariateDistribution>> tau2_dists;
    std::optional<MultivariateDistribution> tau2_dist;

    Parameters() = default;
  };

  LinearModelStrategy() = default;

  explicit LinearModelStrategy(const Parameters p)
      : params(p) {};

  void genData(Experiment *experiment) override;

  std::vector<arma::Row<float>>
  genNewObservationsForAllGroups(Experiment *experiment,
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
  
  j.at("tau2").get_to(p.tau2);

  if (j.at("measurements").type() == nlohmann::detail::value_t::object) {
    p.m_meas_dist = makeMultivariateDistribution(j.at("measurements"));
    
    j["measurements"]["means"].get_to(p.means);
  }

  if (j.at("measurements").type() == nlohmann::detail::value_t::array) {
    std::vector<UnivariateDistribution> dists;
    for (const auto &value : j["measurements"]) {
      dists.push_back(makeUnivariateDistribution(value));
    }
    p.meas_dists = dists;
  }
  
  if (j.contains("errors")) {
    if (j.at("errors").type() == nlohmann::detail::value_t::object) {
      p.m_erro_dist = makeMultivariateDistribution(j.at("errors"));
    }

    if (j.at("errors").type() == nlohmann::detail::value_t::array) {
      std::vector<UnivariateDistribution> dists;
      for (const auto &value : j["errors"]) {
        dists.push_back(makeUnivariateDistribution(value));
      }
      p.erro_dists = dists;
    }
  }
}

//=================================================================================//

///
/// @brief ⚠️ TO BE IMPLEMENTED!
///
/// A Data Strategy for constructing a general [Structural Equation
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
  LatentDataStrategy() = default;

  void genData(Experiment *experiment) override;

  std::vector<arma::Row<float>>
  genNewObservationsForAllGroups(Experiment *experiment,
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
///   N(abilities[i], 1)`.
/// - β
///
class GRMDataStrategy final : public DataStrategy {
public:
  struct Parameters {

    DataModel name{DataModel::GradedResponseModel};

    //! Number of items
    int n_items;
    
    //! Number of categories
    int n_categories;
    
    //! Indicates the Response Function
    std::string response_function {"Rasch"};

    std::optional<MultivariateDistribution> m_diff_dist;
    std::optional<MultivariateDistribution> m_abil_dist;

    std::optional<std::vector<UnivariateDistribution>> diff_dists;
    std::optional<std::vector<UnivariateDistribution>> abil_dists;

    Parameters() = default;
  };

  GRMDataStrategy() = default;;

  GRMDataStrategy(const Parameters &p);

  void genData(Experiment *experiment) override;

  std::vector<arma::Row<float>>
  genNewObservationsForAllGroups(Experiment *experiment,
                                 int n_new_obs) override;


private:
  Parameters params;

  /// Improvement: This can be replaced by `Random::get<bool>` but I need to
  /// test it first.
  UnivariateDistribution uniform_dist = std::uniform_real_distribution<>{};

  arma::Mat<float> poa;        //! probability of answering
  arma::umat responses; //! responses to items, binary
  arma::Col<float> scores;     //! scores of each item

  arma::Mat<float> urand;

  //! Item difficulties
  arma::Mat<float> betas;

  //! Participants abilities
  arma::Mat<float> thetas;
  
  ///
  /// @brief      [Rasch Response Function](https://en.wikipedia.org/wiki/Rasch_model)
  ///
  /// @param[in]  theta  Indicates the participant's ability, θ
  ///
  /// @return     Sum score of `j`th participant over all items
  ///
  float rasch_score(const float theta);
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
    p.m_diff_dist = makeMultivariateDistribution(j.at("difficulties"));
  } else {
    if (j.at("difficulties").type() == nlohmann::detail::value_t::array) {
      std::vector<UnivariateDistribution> dists;
      for (const auto &value : j["difficulties"])
        dists.push_back(makeUnivariateDistribution(value));
      p.diff_dists = dists;
    }
  }

  // Collecting abilities
  if (j.at("abilities").type() == nlohmann::detail::value_t::object)
    p.m_abil_dist = makeMultivariateDistribution(j.at("abilities"));
  else {
    if (j.at("abilities").type() == nlohmann::detail::value_t::array) {
      std::vector<UnivariateDistribution> dists;
      for (const auto &value : j["abilities"])
        dists.push_back(makeUnivariateDistribution(value));
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
