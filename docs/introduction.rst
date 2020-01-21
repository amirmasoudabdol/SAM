.. _chap-intro:

Introduction
============

Before I discuss SAM’s implementation and design, let’s review the
underlying process of defining, conducting and reporting a common
scientific research. In the coming section, I walk through a 
*simplified* process of producing a scientific result. This will allow me
to expose typical components and entities that are typically involved in each stage. 

In the `next section <design.rst>`__, I will explain how each subprocess is being represented in SAM.

.. _intro-research-process:

Scientific Research Process
---------------------------

The *process* of producing a scientific research/study is often a
cumbersome and complicated process. Despite all different ways of conducting
a research, a typical scientific study starts by formulating a hypothesis
when the `Researcher <design.rst#design-researcher>`__ jots down his ideas about
how a certain process explains a certain phenomenon or how he/she thinks a
complicated system works. To test his hypothesis, he/she *prepare an
experiment* which envelops the underlying parameters,
compromises or criteria of his/her theory.

After the `Experiment Setup <design.rst#design-experiment-setup>`__ is
finalized, an `Experiment <design.rst#design-experiment>`__ is being
conducted by which the researcher collects certain type and amount of data in order to test his/her hypothesis., e.g., censors data, images, etc. 

The next step is a mixture of processing the data, analyzing the data and finally coming up with a conclusion regarding the initial
hypothesis -- regardless of agreeing or disagreeing with the *initial hypothesis*, i.e., *pre-registered hypothesis*.

If results are informative — despite the correctness of the initial hypothesis — the researcher selects a `Journal <design.rst#design-journal>`__ and submit the outcome in the form
of a `Manuscript <design.rst#design-submission>`__ to be reviewed according to the journal’s criteria. Finally, the Journal will decide if the submitted result worth publishing or not.

.. _intro-meta-analysis:

Meta-analysis
~~~~~~~~~~~~~

As it has been shown and discussed [cite, cite], the aforementioned process is often long and prune to errors in every different stage. Natural
complexity of conducting the research as well as large numbers of degrees of freedom at the hand of Researchers and Journals will affect the quality of research and publications. Researchers — intentionally or unintentionally — may make mistakes and journals may select a particular set of publications based on wrong or outdated criteria. As a result, various biases may accumulate
into the publications pool and affect our understanding of a certain topic [cite, cite, cite].

Studying this complicated interaction between a *Researcher*, his/her
*Research*, the *Journal*, i.e. publishing medium, and finally the
outcome, *Knowledge*, is proven to be a challenging task [Marjan, Robbie, Jelte, cite, cite, cite]. In
recent years, the field of meta-analysis has grown in popularity to try
to evaluate and aggregate our collective knowledge of certain
hypotheses. As a result, we have discovered the effect of researchers’
degrees of freedom on published results [cite] and journal’s bias toward
“positive and novel” results on our knowledge [cite]. For instance, it
has been shown that the process of adding new data points to an
experiment — after the initial data collection — is more widespread and
has more severe effects on researchers’ ability to achieve significant
results, and therefore makes his/her research more appealing to biased
journals. Therefore, the aggregated outcome, i.e., *published results*,
of a specific theory will be skewed toward incorrect conclusions.

Unfortunately, studying and understanding effects of some of the other
questionable research practices are not often as straightforward
or apparent [cite, cite] as the effect of *optional stopping*. The list of degrees of freedoms is long [cite], and researchers are not fully aware of the
consequences of certain procedures on the final outcome of their research. At the same time,
the scientific community also do not fully understand (or agree on) the
propagative impacts these practices on our collective knowledge [cite, papers
supporting some of the qrps], a.k.a, *true effect size*.