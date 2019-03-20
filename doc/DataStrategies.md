---
layout: default
title: Data Strategies
nav_order: 8
---

# Data Strategies
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC 
{:toc}

---

`DataStrategy` provides data for the `Experiment`. A `Researcher` during the process of [preparing the research](#ExecutionFlow.md#prepare-research) reaches to the `DataStrategy` object and based on the underlying model and parameters of the `Experiment` — stored in `ExperimentSetup` — populates its `measurements` variable. With this routine, we have tried to simulate the *process of collecting data* in a scientific research.

Since `DataStrategy` is aware of all underlying model, parameters and distribution, it can provide data at any points during the simulation. One important implication of this is during the [optional stopping](HackingStrategies.md#optional-stopping) where `Researcher` needs to collect *new* data from the same population he started from.

You can select the type of your model by setting `--data-strategy` parameters of the [config file](ConfigurationFileSpecficications.md). Two available options are *Linear Model* and *Latent Model*. Based on your model of choice, SAM only uses fractions of parameters provided in the configuration file. In this section, we discuss how SAM uses the parameters to set up the model and how it generates data.

## Linear Model

SAM determines the total number of groups, $n_g$ (internal variable), by multiplying the number of treatment conditions, $n_c$, by the number of dependent variables in each condition, $n_d$. After knowing the number of groups, each group is being populated by $n_o$ observations from a multi-variate normal distribution with the mean of $\mu$ and covariance matrix of $\sigma$, $X \sim MN(\mu, \Sigma)$.

You can change the parameters accordingly to implement your own design. For example, in order to set up an experiment with no covariance, you may set the `--covs` to `0`. This will prompt SAM to only use the diagonal row of the covariance matrix. 

| **Parameters** | **Value** |
|:--|:--|
| `--n-conditions` | $n_c$, `int` |
| `--n-dep-vars` | $n_d$, `int` |
| `--n-obs` | $n_o$, `int` |
| `--means` | $\mu$, `double` or `array` |
| `--vars` | $\sigma^2$, `double` or `array` | 
| `--covs` | $\Sigma$, `double`, `matrix` |

## Latent Model


| **Parameters** | **Value** |
|:--|:--|
| `--n-items` | $n_i$, `int`, or `array` |
| `--loadings` | $\lambda$, `double` or `array` |
| `--err-vars` | $\epsilon_\mu$, `double` or `array` |
| `--err-covs` | $\epsilon_\sigma$, `double` |

