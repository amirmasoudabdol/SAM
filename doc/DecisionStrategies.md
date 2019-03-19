---
layout: default
title: Decision Strategy
nav_order: 7
---

# Decision Strategies
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC 
{:toc}

---

Decision strategy is one of the important components of SAM. It tries to implement `Researcher`'s intuition and logic during different stages of the research. While I have briefly covered the [decision making process](Components.md#deicion-making) before, this section lists all the available methods and clarifies their implementation details. 

The main responsibility of `DecisionStrategy` is to notify the `Researcher` about the state of the results, i.e., whether a significant result found or whether the hacking process should be continued. A researcher can make these decisions in different stages, e.g., after running the test for the first time, during the hacking process, after completing one hack, and just before applying his study to a `Journal`. 

The `DecisionStrategy` class distinguishes between these stages by using a `DecisionStage` variable that can accept four different values. 

A `DecisionStrategy` also needs to know what is researcher’s goal. List of available options for `DecisionPreference` is:

- `PreRegisteredOutcome`
- `MinPvalue`
- `MinSigPvalue`*
- `MaxEffect`
- `MaxSigPvalue`*
- `MaxEffectMinPvalue`*

In the case or `PreRegisteredOutcome`

## HonestDecisionMaker


## ImpatientDecisionMaker

Impatient decision maker algorithm imitates the behavior of an impatient researcher who stops his efforts as soon as he find a significant result. You can specify 


## PatientDecisionMaker




