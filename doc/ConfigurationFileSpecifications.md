---
layout: default
title: Configuration File
nav_order: 5
---

# Configuration File Specification

SAM uses a [JSON](https://www.json.org) file to load and save all simulation parameters. The code block below shows a general configuration file used by SAM to prepare the simulation and all its components. As you can see, the file is separated into 4 different sections, each corresponding to one of SAM's components. After customizing your own configuration file, you can load it to SAM using `./SAMpp --config=your-configuration-file.json`. This will start the simulation as described in [Flow](#ExecutionFlow.md) section.

While most parameters are self-explanatory, this section goes into more details on how SAM will process and interpret them during the initialization phase.


```json
{
  "Simulation Parameters":{
    "--debug": true,
    "--verbose": true,
    "--progress": false,
    "--n-sims": 1,
    "--master-seed": 42,
    "--save-output": true,
    "--output-path": "outputs/",
    "--output-prefix": "auto"
  },
  "Experiment Parameters": {
    "--meta-seed": 42,
    "--data-strategy": "Linear Model",
    "--n-conditions": 1,
    "--n-dep-vars": 3,
    "--n-items": 3,
    "--n-obs": 25,
    "--means": 0.25,
    "--vars": 0.5,
    "--covs": 0.00,
    "--loadings": 0.7,
    "--err-vars": 0.01,
    "--err-covs": 0.001,
    "--test-strategy": {
      "name": "TTest",
      "side": "Two Side",
      "alpha": 0.05
    }
  },
  "Journal Parameters" :{
    "--pub-bias": 0.95,
    "--journal-selection-model": "Significant Selection",
    "--max-pubs": 20,
    "--alpha": 0.05,
    "--side": 0
  },
  "Researcher Parameters": {
    "--is-phacker": true,
    "--p-hacking-methods": [
        [
        	{
	          "name": "Optional Stopping",
	          "mode": "Recursive",
	          "level": "dv",
	          "num": 3,
	          "n_attempts": 3,
	          "max_attempts": 10
	        }
	    ]
    ],
    "--decision-strategy": {
      "name": "Impatient Decision Maker",
      "preference": "Min P-value"
    }
  }
}
```



## Simulation Parameters

This section specifies general parameters of the simulation. These parameters are not necessarily influencing SAM's components but will define the overall behavior of SAM regarding input and output. 

| Parameter         | Value    | Description                                                      |
|:------------------|:-------- |:-----------------------------------------------------------------|
| `--debug`         | `bool`   | Runs SAM in debug mode.        				      |
| `--verbose`       | `bool`   | Causes SAM to be verbose, announcing the execution of different processes.   |
| `--progress`      | `bool`   | Shows the progress bar.										  |
| `--master-seed`   | `int`    | An integer for initiating seed's of the *main random number generator stream*. All other necessary streams will be seeded based on the given seed. Setting this to `"random"` tells SAM to use the clock to randomize the random seed. (default: `42`) |
| `--n-sims`        | `int`    | Number of simulation repeated simulation for given parameters.          |
| `--save-output`   | `bool`   | Tells SAM to export the simulation data to a CSV file.
| `--output-path`   | `string` | A path for output files.								  |
| `--output-prefix` | `string` | A prefix to be added to output filenames. {: .label} Raw simulation data files ends with `_sim.csv`, and meta-analysis data files ends with `_meta.csv` |

## Experiment Parameters

This section lists necessary parameters of the [`ExperimentSetup`](Components.md#experiment-setup) and [`Experiment`](Components.md#experiment). 
With `--means` and other similar variables, if a single numeric value is provided, SAM sets the mean of each group to the given value. On the other hand, if an `array` is provided, mean's of `i`th group will set to `i`th elements of the given array. Similarly, if the parameter refers to a matrix, a single numeric value will initialize the entire matrix with the given value, while providing a `matrix` will set each value individually.

> **Note:** The size of an given `array` or `matrix` must agree with the number of conditions, dependant variables, and items, otherwise an error will occur.

| Parameter         | Value              | Description                                                      |
|:------------------|:-------------------|:-----------------------------------------------------------------|
| `--data-strategy` | `string`           | Specify the underlying data model. See [Data Strategy](DataStrategies.md)    |
| `--n-conditions`  | `int`              | Number of treatment conditions, `nc` .*Excluding the control group.* 				|
| `--n-dep-vars`    | `int`              | Number of dependent variables in each condition, `nd`. 					|
| `--n-items`       | `int`              | Number of items. Only applicable for Latent Model, `ni`.                                                                                                                                                                                                                                                                                                                                                                                                                     |
| `--n-obs`         | `int`, `array`     | Number of observation per group.                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| `--means`         | `double`, `array`  | An array of size `nc * nd`, or a numeric value. |
| `--vars`          | `double`, `array`  | An array of size `nc * nd`, or a numeric value. Diagonal values of *covariance matrix* will set by the given array or value.    |
| `--covs`          | `double`, `martix` | A matrix of size `(nc * nd) x (nc * nd)`. If non-zero, non-diagonal values of *convariance matrix* will set with the given value. |
| `--loadings`      | `double`, `array`  |					|
| `--err-vars`      | `double`, `matrix` |					|
| `--err-covs`      | `double`, `matrix` |					|

## Researcher Parameters

This section defines the behavior of the `Researcher`.

| Parameter               | Value   | Description                                                    |
|:------------------------|:-------|:-----------------------------------------------------------------|
| `--is-phacker`          | `bool` | Indicates whether the `Researcher` is a *hacker* or not, if `true`, the list of hacking strategies will be applied on the `Experiment`. |
| `--decision-strategy` | `dict` | Specification of a `DecisionStrategy`. Read more [here](#DecisionStrategy.md). |
| `--p-hacking-methods`.  | `array` of `array` of `dict` | A list of `list`, each indicating a chain of `HackingStrategy`. Read more [here](#HackingStrategies.md). |


## Journal Parameters

This section specifies the properties of the `Journal`.

| Parameter                   | Value    | Description                                                      |
|:----------------------------|:-------- |:-----------------------------------------------------------------|
| `--pub-bias`                | `double` | Publication bias rate.                                                                                                                                     |
| `--journal-selection-model` | `string` | The `SelectionStrategy` of the journal. Read more [here](#selection-strategies.md).                                                                                                                                |
| `--max-pubs`                | `double` | Maximum number of publications that will be accepted by the `Journal`.                                                                                            |
| `--alpha`                   | `double` | Journal's significance $\alpha$.                                                                                                                           |
| `--side`                    | `int`    | Indicates journal's preference regarding the effect size. Acceptance of Positive/Negative/Neutral results will be indicated by 1, -1, and 0, respectively. |


#### Crash Course on JSON

> A JSON object is an *unordered* set of name/value pairs inserted between two curly brackets, `{"name": "Sam"}`. A JSON list/array is an ordered set of values between two brackets, `[1, "blue", {"name": "Sam"}]`