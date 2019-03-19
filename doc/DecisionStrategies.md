---
layout: default
title: Decision Strategies
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

The main responsibility of the `DecisionStrategy` is to notify the `Researcher` about the state of the results, i.e., whether a significant result exists or whether the hacking process should be continued. 

One important aspects of the decision-making process is the stage where the decision is being made. The `Researcher` can make his/her decision (1) right after running the *first* test (2) during the hacking process (3) after completing one hack. and (4) just before applying his study to a `Journal`. Various decision strategies can behaves differently on every stage and therefore achieve different results. The `DecisionStrategy` class distinguishes between these stages by using a `DecisionStage` variable that can accept four different values. Read more [here](api.md#class_decision_strategy).

Another aspect of the `DecisionStrategy` is how a `Researcher` select a specific outcome variable between all available variables. You can see this as *researcher's goal*, especially when p-hacking methods are involved. `Researcher` needs to whether he is going to only concern about the significance of the pre-registered outcome or he *prefers* outcomes with minimal *p*-values! SAM comes with a set of pre-defined parameters, `DecisionPreference`, as follow:

- **`PreRegisteredOutcome`**. Only reporting the pre-registered outcome
- **`MinPvalue`**. Selecting the outcome with minimum *p*-value.
- **`MaxEffect`**. Selecting the outcome with maximum effect size.
- **`MaxEffectMinPvalue`**. Selecting the *significant* outcome with maximum effect size.

In the case or `PreRegisteredOutcome`, the Researcher always reports the pre-registered result even if it applies some of the p-hacking methods; therefore, his attempts would be to achieve significant results on the pre-registered outcome.

## Honest Decision Maker

A "honest researcher" always reports the pre-registered result and will not apply any p-hacking methods on the Experiment. 

Default
{: .label .label-blue}
*This is the **default** decision making method.*

## Impatient Decision Maker

An "impatient researcher" stops his effort as soon as he/she finds a significant result. This could be before, during or after applying p-hacking methods. You can specify an *impatient decision maker* by setting the `--decision-strategy` value as follow:

```JSON
{
  "type": "ImpatientDecisionMaker",
  "preference": dp
}
```

## Patient Decision Maker

A "patient researcher" behaves differently from the impatient researcher. While he checks the progress of his effort, i.e., applying different methods, he does not make a conclusion until he tries everything. He collect all significant results and based on his preferences reports the best of them. For instance, specification below defines a researcher who — after applying all hacking methods — reports the outcome with largest effect size, `MaxEffect`.


```JSON
{
  "type": "PatientDecisionMaker",
  "preference": dp
}
```




