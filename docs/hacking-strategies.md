Hacking Strategies {#chap-hacking-strategies}
==================

> *Hacking Strategy encapsulates the processes or procedures of
> performing different questionable research practices.*

SAM can execute a complicated set of hacking strategies on a given
`Experiment`. A list of hacking strategies will be applied one-by-one
and in the given order. `Researcher` can observe their influences on all
outcome variables and decides when to stop, and what to report. See
[here](design.rst#design-hacking-strategy) and
[here](flow.rst#flow-perform-research).

The configuration below shows how you can define a chain of hacking
strategies. The `p_hacking_methods` is a *list of list of JSON objects*,
see [crash course on
JSON](configuration-file.rst#config-file-crash-course-on-json). You can
define several groups of hacking strategies, each listing different
hacking strategies with different orders and parameters. Since JSON
arrays' are ordered objects, this setup is suitable for controlling the
order in which each group, and method in each group will be applied on
the `Experiment`. The example below defines 3 groups of hacking
strategies with different numbers of methods in each.

``` {.json}
{
  "is_hacker": true,
  "p_hacking_methods": [
    [ 
       {
           "name": "H1"
       },
       {
           "name": "H2"
       }
    ],
    [
       {
           "name": "H2"
       }
    ],
    [
       {
           "name": "H3"
       },
       {
           "name": "H4"
       },
       {
           "name": "H1"
       }
    ]
  ]
}
```

A JSON object defines the specifications of each hacking strategy. The
code below defines *H1* hacking with three parameters. During the
[initialization phase](flow.rst#flow-initialization), SAM reads the
specification and prepares the hacking strategy accordingly.

``` {.json}
{
  "name": "H1",
  "param_1": 1,
  "param_2": "yellow",
  "param_3": "at all cost"
}
```

The rest of this section lists all hacking strategies implemented in
SAM. If you are interested in developing your own methods, head to
`chap-extending-sam`{.interpreted-text role="doc"} chapter.

Optional Stopping {#hacking-strategies-optional-stopping}
-----------------

Optional stopping is the practice of adding new observations to
`Experiment`'s data after the initial data collection. The incentive
behind applying this method is often to achieve significant results. A
researcher can perform this method in many way. Here we have tried to
provide a set of parameters that can collectively mimic as many
scenarios as possible.

In order to add optional stopping to your list of hacking, you must
replace one of the hacking methods, $h_i$ with the code block below.

``` {.json}
{
  "name": "Optional Stopping",
  "num": 3,
  "attempts": 3,
  "max attempts": 10,
  "level": "dv"
}
```

The optional stopping algorithm is implemented based on the fact that
often a researcher performs multiple attempts to achieve significance.
Here, `attempts` defines the number of attempts and `num` specifies the
number of items --- to be added --- in each attempt.

::: {.tabularcolumns}
Y{0.2}
:::

  -----------------------------------------------------------------------
  **Parameters**          **Type**                **Description**
  ----------------------- ----------------------- -----------------------
  `num`                   *n*, `int`              Number of observations
                                                  to be added on each
                                                  attempt.

  `attempts`              *t*, `int`              Number of attempts
                                                  before stopping the
                                                  process.

  `max_attempts`          *m*, `int`              Maximum number of
                                                  attempts

                          "dvs"                   Adding new values to
                                                  dependent variables.
  -----------------------------------------------------------------------

You can control the intensity of optional stopping by alternating the
available parameters. For instance, you can implement an *extreme*
optional stopping by setting `num = 1` and using large values for
`attempts` and `max_attempts`.

::: {.note}
::: {.title}
Note
:::

As discussed in the `data-strategies`{.interpreted-text role="ref"}
section, optional stopping utilizes the `DataStrategy` for generating
new data points.
:::

Outliers Removal {#hacking-strategies-outliers-removal}
----------------

Removing outliers is another popular researcher's degrees of freedom.
While use of outliers removal is not prohibited, most researcher are not
aware of its consequences.

Outliers removal method can be implemented in several different ways as
well. In the simplest case, a researcher will decide to remove a data
point from a dataset if the value is further than a distance from the
sample mean. One common method is to compare the distance of a value to
different multiplier of standard deviation. This type of outliers
removal can be deployed by defining the following parameters.

``` {.json}
{
  "name": "SD Outlier Removal",
  "num": 2,
  "n_attempts": 3,
  "max_attempts": 10,
  "min_observations": 20,
  "multipliers": [3, 2, 1]
}
```

The main body of outliers removal algorithm is implemented similarly to
the optional stopping. The researcher remove add $n$ items in
`n_attempts` before stopping the process, or achieving significant
results. You can also specify a list of `multipliers`. The algorithm
performs *t* attempts to remove *n* outliers from a dataset based on
given multipliers, $\sigma_i$. The algorithm will advance if there is no
item left to be removed at $i < n$ attempts, or after *n* attempts.

::: {.tabularcolumns}
Y{0.2}
:::

  -----------------------------------------------------------------------
  **Parameters**          **Type**                **Details**
  ----------------------- ----------------------- -----------------------
  `num`                   *n*, `int`              Number of items to be
                                                  removed at each attempt

  `n_attempts`            *t*, `int`              Number of attempts to
                                                  remove outliers for
                                                  each multiplier

  `max_attempts`          `int`                   Maximum number of
                                                  iterations before
                                                  stopping the process.

  `min_observations`      `int`                   The minimum number of
                                                  observations. Outliers
                                                  removal stops removing
                                                  values when a group
                                                  reaches
                                                  `min_observation`.

  `multipliers`           `array`                 A list of multipliers
                                                  to be used.

  `order`                 max first, random       
  -----------------------------------------------------------------------

You can achieve different variants of outliers removal method by
modifying its parameters. For instance, setting `num = 1` and choosing
large values for `n_attempts` will remove the outliers one-by-one from
`Experiment`. You can control this process by specifying a list of
`multipliers`.

Group Pooling {#hacking-strategies-group-pooling}
-------------

Group pooling is the act of pooling data from two or more groups into
one *new* group and compare the newly formed treatment group with the
control group. Group pooling can be applied by adding the following JSON
object to the list of hacking methods.

``` {.json}
{
  "name": "Group Pooling",
  "num": 2
}
```

Group pooling algorithm can pool different number of groups, `num`,
together in order to form a new group. In its current setup, the
Researcher traverses through every permutation of length `num` and
create a new group. When, s/he collected all the combinations, s/he will
then ask the `DecisionStrategy` for its `verdict` and consequently
checks the significance of every new group.

::: {.tabularcolumns}
Y{0.2}
:::

  **Parameters**   **Type**   **Details**
  ---------------- ---------- ------------------------------------------------------
  `num`            `int`      Indicates the number of groups to be pooled together

Pre-processing {#hacking-pre-processing}
--------------

> Almost all hacking strategies mentioned in this chapter can be used as
> pre-processing method, the only difference being that pre-processing
> methods are being applied on the data \[, or design\] before the
> `Experiment` is being passed on to the `Researcher`.
