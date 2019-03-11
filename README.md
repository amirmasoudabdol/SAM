# SAM

SAM is an extensible 

## SAM’s Compartments

SAM consists of 3 main compartments, *Experiment, Researcher* and *Journal*. 

- The *Experiment* contains all the necessary data and parameters for defining and modeling a specific experiment. *ExperimentSetup* is an object inside the Experiment acting as a container for all the parameters. Parameters in the ExperimentSetup cannot be changed and only be set at the start of the experiment. In another word, they act as they are pre-registered experiment. 
- The *Researcher* imitates the behaviors of a researcher and includes his QRP’s preferences. A researcher will collect the data for an Experiment based on the parameters specified in ExperimentSetup. It’ll then run the test, decides which outcome it’s like to submit to the Journal. A researcher action can be altered by his choice of p-hacking method. 
- The *Journal* defines the behavior of the journal or a research track when it comes to reviewing or accepting a publication. 

## Simulation Setup

In order to setup a simulation, SAM needs to know the specific representation of each of its components. All the necessary parameters can be listed in a JSON file and provided to SAM via the `--config` parameters of the CLI, e.g. `./SAM --config=simulation_parameters.json`

### Configuration File

A configuration file consists of 4 parts, each specifying parameters for different part of the simulation. The *Simulation Parameters* specifies the general parameters for the simulation, e.g., the output filename. The *Experiment Parameters* section lists the necessary parameters of the underlying experiment of the simulation, e.g., number of observations, factor loadings. The *Researcher Parameters* indicates the behavior of the researcher, e.g., weather he is a hacker or not and finally the *Journal Parameters* indicates how the journal will review and accept a publication. 


| Parameter | Value | Description |
|:--|:--|:--|
| **Simulation Parameters** | | |
| `--debug`  | `bool` | Indicates if SAMpp is running in debug mode. |
| `--verbose` | `bool` | Cause SAM to be verbose, announcing the execution of different procedures. |
| `--progress` | `bool` | Shows the progress bar. |
| `--master-seed` | `int` | An integer for initiating the seed of the main random number generator stream. All other necessary streams will be seeded based on the given seed.<br> The value **0** tells SAM that master seed should be randomized as well. | 
| `--n-sims`| `int` | Number of simulation with each given set of parameters. |
| `--output-path` | `string` | A path to save the output files |
| `--output-prefix` | `string` | A prefix to be added to output filenames'. All output files will end with `_sim.csv` |
| **Experiment Parameters** | | |
| `--data-strategy`| `string` | See also, DataStrategy.md |
| `--n-conditions`| \f$n_c\f$, `int` | Number of treatment conditions |
| `--n-dep-vars`| \f$n_d\f$, `int` | Number of dependent variables |
| `--n-items`| \f$n_i\f$, `int` | Number of items. Only applicable for Latent Model. |
| `--n-obs`| nobs, `int` | Number of observation per each group |
| `--means`| mu, `double` or `array` | Mean of each group. If a `double` is provided, it'll be broadcasted to `nc * nd` array, therefore all groups will have the same `mu`. If an `array` is given, `mu[i]` will be used for group `i`.  |
| `--vars`| sd, `double` or `array`  | Variance of each group. If a `double` is provided, it'll be broadcasted to `nc * nd` array, therefore all groups will have the same `sd`. If an `array` is given, `sd[I]` will be used for group `i`. |
| `--is-correlated`| `bool` | Indicates whether dependent variables are correlated or not. |
| `--covs`| cov, `double` or `2d array` | The covariance coefficient between each group. If a `double` is provided, it'll be broadcasted to a matrix of  `nc * nd` by `nc * nd` with `cov` for every `i` and `j`, therefore all groups will have the same covariance. If a `2d array` is given, `cov[I][j]` will indicate the covariance coefficient between group `i` and `j`.<br> **Note:** If both `sd` and `cov` are scalar values, diagonal elements of `cov` matrix will be replaced by `sd` for each `i`. |
| `--loadings`|  | |
| `--err-vars`| | |
| `--err-covs`| | |
| **Researcher Parameters** | | |
| `--is-phacker` | `bool` | Whether the Researcher is a hacker or not, if `true`, listed methods will be applied on the dataset. |
| `--p-hacking-methods` | `list` of `dict` | A list of `dict` each indicating a Hacking Method with its parameters. |
| **Journal Parameters** | | |
| `--pub-bias` | `double` | Publication bias rate. |
| `--journal-selection-model` | `string` | See also, Selection Model |
| `--max-pubs` | `double` | Maximum publications that is going to be accepted by a Journal. |
| `--alpha` | \f$\alpha\f$, `double` | Alpha of the significant testing |
| `--side` | `int` | Indicates journal's preference regarding the effect size. Acceptance of Positive/Negative/Neutral results will be indicated by 1, -1, and 0, respectively. |


### QRP Methods

While SAM provides an interface to implement your own QRP algorithms, it will provide a few methods out of the box.

Read more on ...


## Outputs

