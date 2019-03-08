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

#### Outcome Switching

Outcome switching is the act of reporting a different outcome variable instead of the *pre-registered outcome*. By default, the first group is considered a pre-registered group, and its the outcome that is going to be chosen and reported to the Journal. However, *Researcher's preference* for reporting the pre-registered outcome can be overwritten by adding a _Outcome Switching_ method to the list of hacking methods.

```
{
    "type": "Outcome Switching",
    "preference": "Min P-value"
}
```

| Parameters | Values | Descriptions  |
|:--|:--|:--|
| `preference` | "Pre-registered Outcome" | The Researcher will only choose and report the pre-registered outcome to the Journal. |
|  | "Min P-value" | The outcome with minimum *p*-value will be observed and reported to the Journal.  |
|  | "Max Effect" | The outcome with maximum effect size is going to be observed and reported to the Journal. |
|  | "Max Effect, Min P-value" | The outcome with maximum effect and minimum p-value will be observed and reported to the Journal. |



#### Optional Stopping

Optional stopping is the practice of adding new observation to the dataset until significant achievement.

```
{
	"type": "Optional Stopping",
	"mode": "Extreme",
	"num": n,
	"attempts": t,
	"max attempts": m,
	"level": "dv"
}
```

| **Parameters** | **Value** | **Description** |
|:--|:--|:--|
| `mode` | "Extreme" | In each attempt, the algorithm adds ***n = 1*** observation to the dataset until it achieves significance. Both `attempts` and `num`  will be ignored in this mode. The process will stop after `max attempts` regardless of achieving significant.  |
| | "Recursive" (*Default*) | In *t* attempts, the algorithm adds *n* observations to the dataset until it achieves significance. |
| `num` | _n_, `int`  | Number of observations to be added on each attempt. |
| `attempts` | _t_, `int` | Number of attempts before stopping the process. |
| `max attempts` | m, `int`  | Maximum number of attempts |
| `level` | "item" | Adding new value to items in the underlying SEM. <br>**Note:** Only applicable in Latent Model. |
|  | "dvs" | Adding new values to dependent variables.  |

#### Outlier Removal

Removing outliers can be done in several different ways. In the simplest case, a researcher will decide to remove a datapoint from a dataset if the value is farther than a distance from the sample mean. One common method is to compare the distance of a value to different multiplier of \sigma. This type of outlier removal can be deployed by defining the following parameters.


```
{
	"type": "SD Outlier Removal",
	"mode": "Extreme",  
	"level": "dv",   
	"num": n,
	"n_attempts": t,
	"max_attempts": m,
	"multipliers": [...]
}
```

| **Parameters** | **Value** | **Details** |
|:--|:--|:--|
| `mode` | "Extreme" | A researcher will remove **_n = 1_** outlier in each trial until he achieves significance. If more than one multiplier is provided, the process starts with the largest multiplier and continues recursively until there is no item can be removed from the dataset using the smallest multiplier. <br>**Note**: In this case, `num` will be ignored. |
| | "Recursive" | At each round, the researcher will remove _n_ outliers based on the given multipliers. He will advance toward smaller multipliers when no item can be removed using the larger values. In this case, `attempts` will be ignored. |
| | "Recursive Attempts" (*Default*) | The algorithm performs *t* attempts to remove *n* outliers from a dataset based on given multipliers. The algorithm will advance if there is no item left to be removed at *i < n* attempts, or after *n* attempts. |
| `num` | *n*, `int` | Number of items to be removed at each attempt |
| `attempts` | _t_, `int` | Number of attempts to remove outliers for each multiplier |
| `multipliers` | `list` | A list of multipliers to be used. |
| `level` | "dv" | Removing outliers at dependent variable level.|
|  | "item" | Removing outliers at the item level, only applicable under Latent Model configuration. |
| `order` | max first, random |  |


#### Group Pooling



## Outputs

