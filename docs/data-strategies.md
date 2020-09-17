Data Strategies {#chap-data-strategies}
===============

> *Data Strategy is the source of data, i.e., population. It knows the
> underlying model, and its properties.*

`DataStrategy` populates the `Experiment`\'s `measurements` variables. A
`Researcher`, during the [preperation
process](flow.rst#flow-prepare-research), reaches to `DataStrategy`
object and based on the underlying model and parameters
populates/generates/collects data for the experiment.

Since `DataStrategy` is aware of all the underlying models, parameters
and distributions, it can provide data at any points during the
simulation. One important implication of this is during the hacking
process. As mentioend, methods like [optional
stopping](hacking-strategies.rst#hacking-strategies-optional-stoppin)
needs to add new data points to the already existing measurements.
Requiring data strategies to produce *new* data points helps with
implementation of such hacking strategies.

You can select the type of model by setting `_name` variables of the
`data_strategy` parameter in the config file. Two available options are
`data-strategies-linear`{.interpreted-text role="ref"} and
`data-strategies-grm`{.interpreted-text role="ref"}. Based on your model
of choice, you must provide different set of variables.

Parameters of the `DataStrategy` are intertwined with the
`ExperimentSetup` parameters. SAM determines the total number of groups,
$n_g$ (internal variable), by multiplying the number of treatment
conditions, $n_c$, by the number of dependent variables in each
condition, $n_d$. After knowing the number of groups, each group is
being populated by $n_o$ observations based on the given model,
specified in `data_strategy`.

``` {.json}
{
"experiment_parameters": {
        "n_conditions": 2,
        "n_dep_vars": 1,
        "n_obs": 25
        "data_strategy": {
            "name": "Model_Name",
            ...
        }
    }
}
```

  **Parameters**   **Value** / **Type**
  ---------------- ----------------------
  `n_conditions`   $n_c$, `int`
  `n_dep_vars`     $n_d$, `int`
  `n_obs`          $n_o$, `int`

  : General `ExperimentSetup` parameters

Linear Model {#data-strategies-linear}
------------

In the case of Linear Model, SAM uses multi-variate normal distribution
with the mean of $\mu$ and covariance matrix of $\Sigma$,
$X \sim MN(\mu, \Sigma)$.

You can change the parameters accordingly to implement your own design.
For example, in order to set up an experiment with no covariance, you
may set the `covs` to `0`. This will prompt SAM to only use the diagonal
row of the covariance matrix.

  **Parameters**   **Value** / **Type**
  ---------------- ---------------------------------
  `means`          $\mu$, `double` or `array`
  `stddevs`        $\sigma^2$, `double` or `array`
  `covs`           $\Sigma$, `double`, `matrix`

  : Parameters of the Linear Model

::: {.important}
::: {.title}
Important
:::

Length of all parameters should complies with the given size of the
study. For instance, if you are planning to run a study with 2 dependent
variables and 2 conditions, you need to specify an array of 4 for the
mean of each group.
:::

::: {.admonition}
Example

``` {.json}
{
    "data_strategy": {
        "name": "LinearModel",
        "means": [0, 0.2],
        "covs": 0.0,
        "stddevs": 1.0,
    }
}
```
:::

Graded Response Model {#data-strategies-grm}
---------------------

Current implementation of Graded Response Model is based on the
generalization of Rasch Model. The probablity of person $j$ answering an
item $i$ correctly, $Pr(X_{ij} = 1)$, can be calculated based on the
difficulty $\beta$ of the item $i$ and the ability $\theta$ of a person
$j$ with the following model \[from Marjan\]:

$$Pr(X_{ij} = 1) = \frac{exp(\theta_j - \beta_i)}{1 + exp(\theta_j - \beta_i)}$$

  **Parameters**   **Value** / **Type**
  ---------------- -------------------------------
  `n_categories`   `int`
  `n_items`        `int`
  `abilities`      $\theta$, `double` or `array`
  `difficulties`   $\beta$, `double` or `array`

  : Parameters of the Graded Response Model

After calculating all responses of person $j$ to all items, the sum
score of all answers is calcuated for each person by adding all the item
scores \[from, Marjan 2014\].

::: {.admonition}
Example

``` {.json}
{
    "name": "GradedResponseModel"
    "data_strategy": {
        "n_categories": 4,
        "n_items": 3,
        "abilities": [0, 0.2],
        "difficulties": 0,
    }
}
```
:::
