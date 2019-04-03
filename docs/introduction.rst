Introduction
============

Before I discuss SAM’s implementation and design, let’s review the
underlying process of defining, conducting and reporting a common
scientific research. In the next section, I will try to explain a
simplified process of producing a scientific result. This will allow me
to expose typical components and entities involved in each stage, and
later on, to explain how each subprocess translates into different
components of SAM. At the end, I will briefly touch up on the question
of why we developed SAM the way we did.

Scientific Research Process
---------------------------

The *process* of producing a scientific research/study is often a
cumbersome and complicated process. Despite all different ways of doing
research, a typical scientific study starts by formulating a hypothesis
as the `Researcher <Components.md#researcher>`__ jots down his ideas on
how one process explains a certain phenomenon or how thinks a
complicated system works. To test his hypothesis, he/she *prepare an
experiment* in which he/she describes the underlying parameters,
compromises or criteria of the study.

After the `Experiment Setup <Components.md#experiment-setup>`__ is
finalized, the `Experiment <Components.md#experiment>`__ is being
conducted in which the researcher collects a set of datasets summarizing
measurements, censors data, images, etc. that are required to test
his/her hypothesis. The next step is a mixture of processing the data,
analyzing the data and coming up with a conclusion regarding the initial
hypothesis regardless of agreeing or disagreeing with the *initial
hypothesis*, i.e., *pre-registered hypothesis*.

If the result is satisfactory and informative, *despite the correctness
of the initial hypothesis*, the researcher selects a
`Journal <Components.md#journal>`__ and submit his research in the form
of a `Paper <Components.md#submission>`__ to be reviewed by journal’s
criteria. Finally, the Journal will decide if the submitted result worth
publishing or not.

Meta-analysis
~~~~~~~~~~~~~

As it has been shown and discussed [cite, cite], this process is often
long and prune to many errors in every different stage. Its natural
complexity as well as large numbers of degrees of freedom in the hand of
Researchers and Journals will affect the quality of research and
publications. Researchers — intentionally or unintentionally — may make
mistakes and Journals may select a particular set of publications based
on wrong or outdated criteria and, as a result, they might introduce
various biases into their publications pool.

Studying this complicated interaction between a *Researcher*, his/her
*Research*, the *Journal*, i.e. publishing medium, and finally the
outcome, *Knowledge*, is proven to be a challenging task [cite]. In
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
questionable research practices, QRPs, are not often as straightforward
or apparent as the effect of *optional stopping*. The list of degrees of
freedoms is long [cite], and researchers are not fully aware of their
consequences on the final outcome of their research. At the same time,
the scientific community is also not fully understand (or agree on) the
propagative impacts these practices on our collective knowledge, i.e.,
*true effect size*.
