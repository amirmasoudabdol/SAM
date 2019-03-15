---
layout: default
title: Hacking Strategies
nav_order: 6
---

# Hacking Strategies
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC 
{:toc}

---

## List of Strategies

SAM can execute complicated set of hacking stratgies on a given `Experiment`. A list of hacking strategies will be applied one-by-one and in the given order. `Researcher` can observe their influences on all outcome variables and decides when to stop, and what to report, see [here](Components.md#hacking-strategy) and [here](ExecutionFlow.md#perform-research).

The code below shows how you can define a chain of hacking strategies. The `--p-hacking-methods` is a *list of list of JSON objects*, [🤯](ConfigurationFileSpecfications.md#crash-course-on-json). You can define several groups of hacking strategies,  each listing different hacking strategies with different order and parameters.  Since JSON arrays' are ordered objects, this setup is suitable for controlling the order in which each group, and method in each group is going to apply on the `Experiment`. The example below defines 3 groups of hacking strategies with different number of methods in each.


```json
{
  "--is-hacker": true,
  "--p-hacking-methods": [
    [ // Group 1
      // {H1}
      ,
      // {H2}
    ],
    [ // Group 2
      // {H2}
    ],
    [
      // {H3}
      ,
      // {H4}
      ,
      // {H1}
    ]
  ]
}

```

A JSON object defines the specifications of each hacking strategy. The code below defines the *H1* hacking with three parameters. During the [initialization phase](ExecutionFlow.md#initialization), SAM reads the specification and prepare the hacking strategy accordingly. 

```json
{
  "type": "H1",
  "param-1": 1,
  "param-2": "yellow",
  "param-3": "at all cost"
}
```


The rest of this section lists all hacking strategies currently implemented in SAM. If you are interested to develop your own methods, go to [Extending SAM](extending-sam.md).

## Optional Stopping

Optional stopping is the practice of adding new observation to the dataset until significant achievement.

```json
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

## Outlier Removal

Removing outliers can be done in several different ways. In the simplest case, a researcher will decide to remove a datapoint from a dataset if the value is farther than a distance from the sample mean. One common method is to compare the distance of a value to different multiplier of \sigma. This type of outlier removal can be deployed by defining the following parameters.


```json
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


## Group Pooling


