---
title: Configuration File
layout: default
---

Configuration File {#chap-config-file}
==================

SAM uses a [JSON](https://www.json.org) file to load and save all
simulation parameters. The code block below shows a general
configuration file used by SAM to prepare the simulation and all its
components. As it\'s shown, the file is separated into 4 different
sections, each corresponding to one of SAM's components. After
customizing your own configuration file, you can load it to SAM using
`./SAMpp --config=your-configuration-file.json`. This will start the
simulation as described in the `flow`{.interpreted-text role="doc"}
section.

While most parameters are self-explanatory, this section goes into more
details on how SAM will process and interpret them during the
initialization phase.

``` {.json}
{
    "simulation_parameters": {
        "debug": false,
        "master_seed": 42,
        "n_sims": 5,
        "output_path": "../outputs/",
        "output_prefix": "",
        "progress": false,
        "save_pubs": true,
        "save_rejected": true,
        "save_sims": false,
        "save_stats": true,
        "verbose": true
    },
    "experiment_parameters": {
        "n_conditions": 2,
        "n_dep_vars": 1,
        "n_obs": 25,
        "data_strategy": {
            "name": "LinearModel",
            "covs": 0.5,
            "means": [
                0,
                0.2
            ],
            "stddevs": 1.0
        },
        "test_strategy": {
            "name": "TTest",
            "alpha": 0.05,
            "side": "TwoSided"
        },
        "effect_strategy": {
            "name": "MeanDifference"
        }
    },
    "journal_parameters": {
        "max_pubs": 20,
        "selection_strategy": {
            "name": "FreeSelection"
        }
    },
    "researcher_parameters": {
        "decision_strategy": {
            "name": "PatientDecisionMaker",
            "preference": "MarjansHacker",
            "publishing_policy": "Anything"
        },
        "hacking_strategies": [
            [
                {
                    "name": "SubjectiveOutlierRemoval",
                    "min_observations": 5,
                    "range": [
                        0.5,
                        4
                    ],
                    "step_size": 0.1
                }
            ]
        ],
        "is_phacker": true,
        "is_pre_processing": false,
        "pre_processing_methods": [
            {
                "name": "OutliersRemoval",
                "level": "dv",
                "max_attempts": 1000,
                "min_observations": 10,
                "multipliers": [
                    0.5
                ],
                "n_attempts": 1000,
                "num": 1000,
                "order": "random"
            }
        ]
    }
}
```

Simulation Parameters {#config-file-simulation-params}
---------------------

This section specifies general parameters of the simulation. These
parameters are not necessarily influencing SAM components\'. They define
the overall behavior of SAM regarding input and output.

::: {.tabularcolumns}
Y{0.2}
:::

  ------------------------------------------------------------------------
  **Parameter**     **Type**   **Description**
  ----------------- ---------- -------------------------------------------
  `debug`           `bool`     Runs SAM in debug mode.

  `verbose`         `bool`     Causes SAM to be verbose, announcing the
                               execution of dfiferent processes.

  `progress`        `bool`     Shows the progress bar.

  `master_seed`     `int`      An integer for initiating seed's of the
                               *main random number generator stream*. All
                               other necessary streams will be seeded
                               based on the given seed. Setting this to
                               `"random"` tells SAM to use the clock to
                               randomize the random seed. (default: `42`)

  `n_sims`          `int`      Number of simulation repeated simulation
                               for given parameters.

  `save_output`     `bool`     Tells SAM to export the simulation data to
                               a CSV file

  `output_path`     `string`   A path for output files.

  `output_prefix`   `string`   A prefix to be added to output filenames.
                               {: .label} Raw simulation data files ends
                               with `_sim.csv`, and meta-analysis data
                               files ends with `_meta.csv`
  ------------------------------------------------------------------------

Experiment Parameters {#config-file-experiment-params}
---------------------

This section lists necessary parameters of the [Experiment
Setup](design.rst#design-experiment-setup) and
[Experiment](design.rst#design-experiment).

::: {.tabularcolumns}
Y{0.2}
:::

+---------------------+----------+-------------------------------------+
| **Parameter**       | **Type** | **Description**                     |
+=====================+==========+=====================================+
| `n_conditions`      | `int`    | Nu                                  |
|                     |          | mber of treatment conditions, `nc`. |
|                     |          |                                     |
|                     |          | :   *Excluding the control group.*  |
+---------------------+----------+-------------------------------------+
| `n_dep_vars`        | `int`    | Number of dependent variables in    |
|                     |          | each condition, `nd`.               |
+---------------------+----------+-------------------------------------+
| `n_items`           | `int`    | Number of items. Only applicable    |
|                     |          | for Latent Model, `ni`.             |
+---------------------+----------+-------------------------------------+
| `n_obs`             | `int`,   | Number of observation per group.    |
|                     | `array`  |                                     |
+---------------------+----------+-------------------------------------+
| `test_strategy`     | `string` | Specify the underlying test         |
|                     |          | strategy.                           |
+---------------------+----------+-------------------------------------+
| `data_strategy`     | `string` | Specify the underlying data         |
|                     |          | strategy.                           |
+---------------------+----------+-------------------------------------+
| `effect_strategy`   | `string` | | Specify the underlying effect     |
|                     |          |   strategy.                         |
+---------------------+----------+-------------------------------------+

::: {.note}
::: {.title}
Note
:::

Each `Data`, `Test`, or `Effect` strategy might carry its own set of
parameters. See, `design`{.interpreted-text role="doc"},
doc:[data-strategy]{.title-ref}, doc:[test-strategy]{.title-ref},
doc:[effect-strategy]{.title-ref} for more info.
:::

::: {.important}
::: {.title}
Important
:::

The size of an given `array` or `matrix` must agree with the number of
conditions, dependant variables, and items, otherwise an error will
occur.
:::

Researcher Parameters {#config-file-researcher-params}
---------------------

This section defines the behavior of the `Researcher`.

::: {.tabularcolumns}
Y{0.2}
:::

  -------------------------------------------------------------------------------------------------------
  **Parameter**              **Type**   **Description**
  -------------------------- ---------- -----------------------------------------------------------------
  `is_phacker`               `bool`     Indicates whether the `Researcher` is a *hacker* or not, if
                                        `true`, the list of hacking strategies will be applied on the
                                        `Experiment`.

  `p_hacking_methods`        `list`     A list of `list`, each indicating a chain of `HackingStrategy`.

  `is_pre_processing`        `bool`     Indicates whether any pre-processing procedure is being performed
                                        on the data before passing the data to the researcher for
                                        analysis.

  `pre_processing_methods`   `list`     Similar to `p_hacking_methods`. See
                                        [Pre-processing](hacking-strategies.rst#hacking-pre-processing)

  `decision_strategy`        `dict`     Specification of a `DecisionStrategy`. See more
                                        `decision-strategies`{.interpreted-text role="doc"}.
  -------------------------------------------------------------------------------------------------------

Journal Parameters {#config-file-journal-params}
------------------

This section specifies the properties of the `Journal`.

::: {.tabularcolumns}
Y{0.2}
:::

  ------------------------------------------------------------------------------
  **Parameter**          **Type**   **Description**
  ---------------------- ---------- --------------------------------------------
  `max_pubs`             `double`   Maximum number of publications that will be
                                    accepted by the `Journal`.

  `selection_strategy`   `string`   The `SelectionStrategy` of the journal.
  ------------------------------------------------------------------------------

::: {.note}
::: {.title}
Note
:::

Parameters like `pub_bias`, `alpha` or `side` can be set based on the
`SelectionStrategy` of user\'s choice. See,
`selection-strategy`{.interpreted-text role="doc"} for more info.
:::

### Crash Course on JSON {#config-file-json}

> A JSON object is an *unordered* set of name/value pairs inserted
> between two curly brackets, `{"name": "S.A.M"}`. A JSON list/array is
> an ordered set of values between two brackets,
> `[1, "blue", {"name": "S.A.M"}]`
