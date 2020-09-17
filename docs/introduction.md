---
title: Introduction
layout: default
---

Introduction
============

Before I discuss SAM's implementation and design, let's review the
common process of defining, conducting and reporting a scientific
research. In the coming section, I will walk through a *simplified*
process of producing a scientific result, this will expose typical
components and entities that are typically involved in each stage.
Later, in the `design`{.interpreted-text role="doc"} chapter, I will
explain how each compartments is going to be represented in SAM.

Scientific Research Process
---------------------------

The *process* of producing a scientific research/study is often a
cumbersome and complicated process. Despite all different ways of
conducting a research, a typical scientific study starts by formulating
a hypothesis when the [Researcher](design.rst#design-researcher) jots
down his/her ideas about how a certain process explains a certain
phenomenon or how s/he thinks a complicated system works. To test this
hypothesis, s/he *prepares an experiment* to test the underlying
parameters, compromises or criteria of his/her theory.

After the [Experiment Setup](design.rst#design-experiment-setup) is
finalized, an [Experiment](design.rst#design-experiment) is being
conducted by which the researcher collects certain types and amount of
data in order to test his/her hypothesis., e.g., censors data, images,
etc.

The next step is a mixture of pre-/processing the data, analyzing the
data and finally coming up with a conclusion regarding the initial
hypothesis. If results are informative --- regardless of agreeing or
disagreeing with the *initial hypothesis*, i.e., *pre-registered
hypothesis* --- the researcher selects a
[Journal](design.rst#design-journal) and submit his/her findings in the
form of a [Manuscript](design.rst#design-submission), to be reviewed
according to the journal's criteria. Finally, the Journal will decide if
the submitted manuscript worth publishing or not. .. \--

### Meta-analysis

As it has been shown and discussed \[cite, cite\], the aforementioned
process is often long and prune to errors in every different stage.
Natural complexity of conducting a research alongside the large numbers
of degrees of freedom available to researchers and journals will
undoubtedly affect the quality of research and publications. Researchers
--- intentionally or unintentionally --- may make mistakes, and journals
may select a particular set of publications based on wrong or outdated
criteria. As a result, various biases will be accumulated into the
publications pool and ultimately affect our understanding of a certain
topic \[cite, cite, cite\].

In recent years, the field of meta-analysis has grown in popularity to
evaluate and account for various biases presented in publications. As a
result, we have discovered the effect of researchers' degrees of freedom
on published results \[cite\] and unwanted outcome of journals\'
tendency to only publish positive and novel results \[cite\]. For
instance, it has been shown that the process of adding new data points
to an experiment --- after the initial data collection --- is a
widespread practice and it has a severe effect on researchers' ability
to find significant results; and therefore makes his/her research more
appealing to biased journals. This will consequently skew our collective
knowledge when being aggregated by journals biased toward significant
results.

Studying this complicated interaction between a *Researcher*, his/her
*Research*, the *Journal*, i.e. publishing medium, and finally the
outcome, i.e., *Knowledge*, is proven to be a challenging task \[Marjan,
Robbie, Jelte, cite, cite, cite\]. The list of degrees of freedoms is
long \[cite\], and researchers are not fully aware of the consequences
of certain procedures on the final outcome of their research. At the
same time, the scientific community also do not fully understand (or
agree on) the propagative impacts of these practices on our collective
knowledge \[cite, papers supporting some of the qrps\], a.k.a, *true
effect size*.

SAM
---

\[Say something about SAM\]
