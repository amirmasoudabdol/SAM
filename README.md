# SAM

### Configuration File

|  **Simulation Parameters** | | |
| Parameter | Value | Description |
|:--|:--:|:--|
| `--debug`  | `bool` | Indicates if SAMpp is running in debug mode. |
| `--verbose` | `bool` | Cause SAM to be verbose, announcing the execution of different procedures. |
| `--progress` | `bool` | Shows the progress bar. |
| `--master-seed` | `int` | An integer for initiating the seed of the main random number generator stream. All other necessary streams will be seeded based on the given seed.<br> The value **0** tells SAM that master seed should be randomized as well. | 
| `--n-sims`| `int` | Number of simulation with each given set of parameters. |
| `--output-path` | `string` | A path to save the output files |
| `--output-prefix` | `string` | A prefix to be added to output filenames'. All output files will end with `_sim.csv` |
| **Experiment Parameters** | | |
| `--data-strategy`| | |
| `--n-conditions`| | |
| `--n-dep-vars`| | |
| `--n-items`| | |
| `--n-obs`| | |
| `--means`| | |
| `--sds`| | |
| `--is-correlated`| | |
| `--covs`| | |
| `--loadings`| | |
| `--err-sds`| | |
| `--err-covs`| | |
| **Researcher Parameters** | | |
| `--is-phacker` | | |
| `--p-hacking-methods` | | |
| **Journal Parameters** | | |
| `--pub-bias` | | |
| `--journal-selection-model` | | |
| `--max-pubs` | | |
| `--alpha` | | |
| `--side` | | |


### QRP Methods

SAM provides a few common QRP methods out of the box. Some of the methods can be configured

#### Outcome Switching

```
{
    "type": "Outcome Switching",
    "preference": "Min P-value"
}
```

#### Optional Stopping

Optional stopping is the practice of adding new observation to the dataset until significant achievement.

```
{
	"type": "Optional Stopping",
	"mode": "Extreme",
	"num": n,
	"attempts": t,
	"max attempts": m,
}
```

| Parameters | Value | Description |
|:--|:--|:--|
| `mode` | "Extreme" | In each attempt, the algorithm adds ***n = 1*** observation to the dataset until it achieves significance. Both `attempts` and `num`  will be ignored in this mode. The process will stop after `max attempts` regardless of achieving significant.  |
| | "Recursive" _(Default)_ | In _t_ attempts, the algorithm adds _n_ observations to the dataset until it achieves significance. |
| `num` | _n_, `int`  | Number of observations to be added on each attempt. |
| `attempts` | _t_, `int` | Number of attempts before stopping the process. |
| `max attempts` | m, `int`  | Maximum number of attempts |

#### Outlier Removal

Removing outliers can be done in several different ways. In the simplest case, a researcher will decide to remove a datapoint from a dataset if the value is farther than a distance from the sample mean. One common method is to compare the distance of a value to different multiplier of \sigma. This type of outlier removal can be deployed by defining the following parameters.


```
{
	"type": "SD Outlier Removal",
	"mode": "Extreme",     
	"num": n,
	"attempts": t,
	"multipliers": [...]
}
```

| Parameters | Value | Details |
|:--|:--|:--|
| `mode` | "Extreme" | A researcher will remove **_n = 1_** outlier in each trial until he achieves significance. If more than one multiplier is provided, the process starts with the largest multiplier and continues recursively until there is no item can be removed from the dataset using the smallest multiplier. <br>**Note**: In this case, `num` will be ignored. |
| | "Recursive" | At each round, the researcher will remove _n_ outliers based on the given multipliers. He will advance toward smaller multipliers when no item can be removed using the larger values. In this case, `attempts` will be ignored. |
| | "Recursive Attempts" _(Default)_ | The algorithm performs _t_ attempts to remove _n_ outliers from a dataset based on given multipliers. The algorithm will advance if there is no item left to be removed at _i < n_ attempts, or after _n_ attempts. |
| `num` | *n*, `int` | Number of items to be removed at each attempt |
| `attempts` | _t_, `int` | Number of attempts to remove outliers for each multiplier |
| `multipliers` | `list` | A list of multipliers to be used. |


#### Group Pooling

